//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// copyright            : (C) 2008 by Eran Ifrah
// file name            : breakpointdlg.cpp
//
// -------------------------------------------------------------------------
// A
//              _____           _      _     _ _
//             /  __ \         | |    | |   (_) |
//             | /  \/ ___   __| | ___| |    _| |_ ___
//             | |    / _ \ / _  |/ _ \ |   | | __/ _ )
//             | \__/\ (_) | (_| |  __/ |___| | ||  __/
//              \____/\___/ \__,_|\___\_____/_|\__\___|
//
//                                                  F i l e
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include "BreakpointsView.hpp"

#include "Debugger/debuggermanager.h"
#include "breakpointslistctrl.h"
#include "event_notifier.h"
#include "file_logger.h"
#include "frame.h"
#include "globals.h"
#include "macros.h"
#include "manager.h"
#include "plugin.h"

BreakpointsView::BreakpointsView(wxWindow* parent)
    : BreakpointTabBase(parent)
{
    EventNotifier::Get()->Bind(wxEVT_BREAKPOINTS_UPDATED, &BreakpointsView::OnBreakpointsUpdated, this);
    EventNotifier::Get()->Bind(wxEVT_SESSION_LOADING, &BreakpointsView::OnSessionLoading, this);
    EventNotifier::Get()->Bind(wxEVT_SESSION_LOADED, &BreakpointsView::OnSessionLoaded, this);
    Initialize();
}

BreakpointsView::~BreakpointsView()
{
    EventNotifier::Get()->Unbind(wxEVT_BREAKPOINTS_UPDATED, &BreakpointsView::OnBreakpointsUpdated, this);
    EventNotifier::Get()->Unbind(wxEVT_SESSION_LOADING, &BreakpointsView::OnSessionLoading, this);
    EventNotifier::Get()->Unbind(wxEVT_SESSION_LOADED, &BreakpointsView::OnSessionLoaded, this);
}

void BreakpointsView::Initialize()
{
    std::vector<clDebuggerBreakpoint> bps;
    clGetManager()->GetAllBreakpoints(bps);

    // This does the display stuff
    m_dvListCtrlBreakpoints->Initialise(bps);

    // Store the internal and external ids
    m_ids.clear();
    for (const auto& bp : bps) {
        struct bpd_IDs IDs(bp);
        m_ids.push_back(IDs);
    }

    bool hasItems = !m_dvListCtrlBreakpoints->IsEmpty();
    if(hasItems) {
        m_dvListCtrlBreakpoints->Select(m_dvListCtrlBreakpoints->RowToItem(0));
    }

    // Since any change results in Initialize() being rerun, we can do updateUI here
    m_buttonEdit->Enable(hasItems);
    m_buttonDelete->Enable(hasItems);
    // The 'Apply Pending' button is more complicated: it should be hidden,
    // unless there are pending bps to apply,and the debugger is running
    bool pending = ManagerST::Get()->GetBreakpointsMgr()->PendingBreakpointsExist();
    IDebugger* dbgr = DebuggerMgr::Get().GetActiveDebugger();
    m_buttonApplyPending->Show(pending && dbgr && dbgr->IsRunning());
    Layout();
    // Enable DeleteAll if there are either bps or pending bps
    m_buttonDeleteAll->Enable(hasItems || pending);
}

void BreakpointsView::OnDelete(wxCommandEvent& e)
{
    wxDataViewItem item = m_dvListCtrlBreakpoints->GetSelection();
    if(!item.IsOk()) {
        return;
    }

    // Delete by this item's id, which was carefully stored in Initialize()
    int id = m_ids[m_dvListCtrlBreakpoints->ItemToRow(item)].GetBestId();
    ManagerST::Get()->GetBreakpointsMgr()->DelBreakpoint(id);

    clMainFrame::Get()->GetStatusBar()->SetMessage(_("Breakpoint successfully deleted"));
    Initialize(); // ReInitialise, as either a bp was deleted, or the data was corrupt
}

void BreakpointsView::OnDeleteAll(wxCommandEvent& e)
{
    wxUnusedVar(e);
    ManagerST::Get()->GetBreakpointsMgr()->DelAllBreakpoints();
    Initialize();

    clMainFrame::Get()->GetStatusBar()->SetMessage(_("All Breakpoints deleted"));

    wxCommandEvent evtDelAll(wxEVT_CODELITE_ALL_BREAKPOINTS_DELETED);
    EventNotifier::Get()->AddPendingEvent(evtDelAll);
}

void BreakpointsView::OnApplyPending(wxCommandEvent& e)
{
    wxUnusedVar(e);
    ManagerST::Get()->GetBreakpointsMgr()->ApplyPendingBreakpoints();
    Initialize();

    clMainFrame::Get()->GetStatusBar()->SetMessage(_("Pending Breakpoints reapplied"));
}

void BreakpointsView::OnBreakpointActivated(wxDataViewEvent& event)
{
    wxString file = m_dvListCtrlBreakpoints->GetItemText(event.GetItem(), m_dvListCtrlBreakpoints->GetFileColumn());
    wxString line = m_dvListCtrlBreakpoints->GetItemText(event.GetItem(), m_dvListCtrlBreakpoints->GetLinenoColumn());
    long line_number;
    line.ToLong(&line_number);

    auto callback = [=](IEditor* editor) {
        editor->GetCtrl()->ClearSelections();
        editor->CenterLine(line_number - 1);
        editor->SetActive();
    };
    clGetManager()->OpenFileAndAsyncExecute(file, std::move(callback));
}

void BreakpointsView::OnEdit(wxCommandEvent& e)
{
    wxDataViewItem item = m_dvListCtrlBreakpoints->GetSelection();
    if(!item.IsOk()) {
        return;
    }

    int row = m_dvListCtrlBreakpoints->ItemToRow(item);
    bool bpExist;
    ManagerST::Get()->GetBreakpointsMgr()->EditBreakpoint(row, bpExist);

    if(!bpExist) {
        // the breakpoint does not exist! remove it from the UI as well
        m_dvListCtrlBreakpoints->DeleteItem(row);
    }

    Initialize(); // Make any changes visible
}

void BreakpointsView::OnAdd(wxCommandEvent& e)
{
    wxUnusedVar(e);

    ManagerST::Get()->GetBreakpointsMgr()->AddBreakpoint();
    Initialize(); // Make any changes visible
}

void BreakpointsListctrl::Initialise(std::vector<clDebuggerBreakpoint>& bps)
{
    DeleteAllItems();
    for(const auto& bp : bps) {

        // Store the internal and external ids
        bpd_IDs IDs(bp);
        wxVector<wxVariant> cols;
        cols.push_back(IDs.GetIdAsString());

        wxString type;
        if (bp.is_temp) {
            type = _("Temp. ");
        }
        type += ((bp.bp_type == BP_type_watchpt) ? _("Watchpoint") : _("Breakpoint"));
        cols.push_back(type);

        wxString disabled;
        if (!bp.is_enabled) {
            disabled = _("disabled");
        }
        cols.push_back(disabled);
        cols.push_back(bp.file);
        cols.push_back((wxString() << bp.lineno));
        cols.push_back(bp.function_name);
        cols.push_back(bp.at);
        cols.push_back(bp.memory_address);
        cols.push_back(bp.what);
        cols.push_back((wxString() << bp.ignore_number));

        wxString extras; // Extras are conditions, or a commandlist. If both (unlikely!) just show the condition
        if (!bp.conditions.IsEmpty()) {
            extras = bp.conditions;
        } else if (!bp.commandlist.IsEmpty()) {
            extras = bp.commandlist;
        }
        if(!extras.IsEmpty()) {
            // We don't want to try to display massive commandlist spread over several lines...
            int index = extras.Find(wxT("\\n"));
            if(index != wxNOT_FOUND) {
                extras = extras.Left(index) + wxT("...");
            }
        }
        cols.push_back(extras);
        AppendItem(cols);
    }
}

void BreakpointsView::OnContextMenu(wxDataViewEvent& event)
{
    wxMenu menu;
    menu.Append(XRCID("edit_breakpoint"), _("Edit Breakpoint..."));
    menu.Append(XRCID("delete_breakpoint"), _("Delete Breakpoint"));

    int where = GetPopupMenuSelectionFromUser(menu);
    if(where == wxID_NONE)
        return;
    if(where == XRCID("edit_breakpoint")) {
        wxCommandEvent dummy;
        OnEdit(event);
    } else if(where == XRCID("delete_breakpoint")) {
        wxCommandEvent dummy;
        OnDelete(event);
    }
}

void BreakpointsView::OnBreakpointsUpdated(clDebugEvent& event)
{
    event.Skip();
    // Update the UI
    Initialize();
}

void BreakpointsView::OnSessionLoading(clCommandEvent& event)
{
    event.Skip();
    m_dvListCtrlBreakpoints->DeleteAllItems();
}

void BreakpointsView::OnSessionLoaded(clCommandEvent& event)
{
    event.Skip();
    Initialize();
}
