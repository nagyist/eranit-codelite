//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// copyright            : (C) 2008 by Eran Ifrah
// file name            : optionsconfig.cpp
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
#include "optionsconfig.h"

#include "cl_defs.h"
#include "editor_config.h"
#include "macros.h"
#include "xmlutils.h"

#include <wx/fontmap.h>
#include <wx/intl.h>

#ifdef __WXMSW__
#include <wx/msw/uxtheme.h>
#endif

const wxString defaultBookmarkLabels = wxString(';', CL_N0_OF_BOOKMARK_TYPES - 1) + "Find";

wxString SetDefaultBookmarkColours()
{
    // (Confusingly, the 'foreground' is actually just the rim of the marker; the background is the central bulk)
    // NB: We want the 'find' colour always to be the most significant, so add any future extra items *before* the last
    // one
    const wxString arr[] = { "#FF0080", "#0000FF", "#FF0000", "#00FF00", "#FFFF00" };
    wxString output;
    for (size_t n = 0; n < CL_N0_OF_BOOKMARK_TYPES; ++n) {
        if (n < sizeof(arr) / sizeof(wxString)) {
            output << arr[n] << ';';
        } else {
            output << "#FF0080" << ';';
        }
    }

    return output;
}

OptionsConfig::OptionsConfig(wxXmlNode* node)
    : m_displayFoldMargin(false)
    , m_underlineFoldLine(false)
    , m_scrollBeyondLastLine(true)
    , m_foldStyle("Arrows")
    , m_displayBookmarkMargin(true)
    , m_bookmarkShape(wxT("Small Arrow"))
    , m_bookmarkBgColours(SetDefaultBookmarkColours())
    , m_bookmarkFgColours(SetDefaultBookmarkColours())
    , m_bookmarkLabels(defaultBookmarkLabels)
    , m_highlightCaretLine(true)
    , m_highlightCaretLineWithColour(true)
    , m_clearHighlightedWordsOnFind(true)
    , m_displayLineNumbers(true)
    , m_relativeLineNumbers(false)
    , m_showIndentationGuidelines(true)
    , m_caretLineColour(wxT("LIGHT BLUE"))
    , m_indentUsesTabs(false)
    , m_indentWidth(4)
    , m_tabWidth(4)
    , m_iconsSize(16)
    , m_showWhitespaces(0 /*wxSCI_WS_INVISIBLE*/)
    , m_foldCompact(false)
    , m_foldAtElse(false)
    , m_foldPreprocessor(false)
    , m_highlightMatchedBraces(true)
    , m_foldBgColour(wxColour(240, 240, 240))
    , m_autoAdjustHScrollBarWidth(true)
    , m_caretWidth(2)
    , m_caretBlinkPeriod(500)
    , m_copyLineEmptySelection(true)
    , m_programConsoleCommand(TERMINAL_CMD)
    , m_eolMode(wxT("Default"))
    , m_trackEditorChanges(false)
    , m_hideOutputPaneOnUserClick(false)
    , m_hideOutputPaneNotIfBuild(false)
    , m_hideOutputPaneNotIfSearch(true)
    , m_hideOutputPaneNotIfReplace(false)
    , m_hideOutputPaneNotIfReferences(false)
    , m_hideOutputPaneNotIfOutput(false)
    , m_hideOutputPaneNotIfTrace(false)
    , m_hideOutputPaneNotIfTasks(false)
    , m_hideOutputPaneNotIfBuildQ(true)
    , m_hideOutputPaneNotIfCppCheck(true)
    , m_hideOutputPaneNotIfSvn(true)
    , m_hideOutputPaneNotIfCscope(false)
    , m_hideOutputPaneNotIfGit(true)
    , m_hideOutputPaneNotIfDebug(true)
    , m_hideOutputPaneNotIfMemCheck(true)
    , m_findBarAtBottom(true)
    , m_showReplaceBar(true)
    , m_TrimLine(true)
    , m_AppendLF(true)
    , m_disableSmartIndent(false)
    , m_disableSemicolonShift(false)
    , m_caretLineAlpha(15)
    , m_dontAutoFoldResults(true)
    , m_dontOverrideSearchStringWithSelection(false)
    , m_findNextOrPreviousUseSelection(true)
    , m_showDebugOnRun(true)
    , m_caretUseCamelCase(true)
    , m_wordWrap(false)
    , m_dockingStyle(0)
    , m_preferredLocale(wxT("en_US"))
    , m_useLocale(0)
    , m_trimOnlyModifiedLines(true)
    , m_workspaceTabsDirection(wxLEFT)
    , m_outputTabsDirection(wxUP)
    , m_indentedComments(false)
    , m_nbTabHeight(nbTabHt_Short)
    , m_webSearchPrefix(wxT("https://www.google.com/search?q="))
    , m_smartParen(true)
{
    m_options.set(Opt_AutoCompleteCurlyBraces);
    m_options.set(Opt_NavKey_Shift);
    m_options.set(Opt_FoldHighlightActiveBlock);
    m_options.set(Opt_TabStyleMinimal);
    m_options.set(Opt_HideDockingWindowCaption);

    m_debuggerMarkerLine = DrawingUtils::LightColour("LIME GREEN", 8.0);
    m_mswTheme = false;
    // set the default font name to be wxFONTENCODING_UTF8
    SetFileFontEncoding(wxFontMapper::GetEncodingName(wxFONTENCODING_UTF8));
    if (node) {
        SetFileFontEncoding(
            XmlUtils::ReadString(node, wxT("FileFontEncoding"), wxFontMapper::GetEncodingName(wxFONTENCODING_UTF8)));

        m_displayFoldMargin = XmlUtils::ReadBool(node, wxT("DisplayFoldMargin"), m_displayFoldMargin);
        m_underlineFoldLine = XmlUtils::ReadBool(node, wxT("UnderlineFoldedLine"), m_underlineFoldLine);
        m_foldStyle = XmlUtils::ReadString(node, wxT("FoldStyle"), m_foldStyle);
        m_displayBookmarkMargin = XmlUtils::ReadBool(node, wxT("DisplayBookmarkMargin"), m_displayBookmarkMargin);
        m_bookmarkShape = XmlUtils::ReadString(node, wxT("BookmarkShape"), m_bookmarkShape);
        m_bookmarkBgColours =
            XmlUtils::ReadString(node, wxT("BookmarkBgColours"), ""); // No default; we'll deal with this later
        m_bookmarkFgColours = XmlUtils::ReadString(node, wxT("BookmarkFgColours"), "");
        m_bookmarkLabels = XmlUtils::ReadString(node, wxT("BookmarkLabels"), defaultBookmarkLabels);
        m_clearHighlightedWordsOnFind =
            XmlUtils::ReadBool(node, wxT("ClearHighlitWordsOnFind"), m_clearHighlightedWordsOnFind);
        m_highlightCaretLine = XmlUtils::ReadBool(node, wxT("HighlightCaretLine"), m_highlightCaretLine);
        m_highlightCaretLineWithColour =
            XmlUtils::ReadBool(node, "HighlightCaretLineWithColour", m_highlightCaretLineWithColour);
        m_displayLineNumbers = XmlUtils::ReadBool(node, wxT("ShowLineNumber"), m_displayLineNumbers);
        m_relativeLineNumbers = XmlUtils::ReadBool(node, wxT("RelativeLineNumber"), m_relativeLineNumbers);
        m_lineNumberHighlightCurrent =
            XmlUtils::ReadBool(node, wxT("LineNumbersHighlightCurrent"), m_lineNumberHighlightCurrent);
        m_showIndentationGuidelines = XmlUtils::ReadBool(node, wxT("IndentationGuides"), m_showIndentationGuidelines);
        m_caretLineColour =
            XmlUtils::ReadString(node, wxT("CaretLineColour"), m_caretLineColour.GetAsString(wxC2S_HTML_SYNTAX));
        m_indentUsesTabs = XmlUtils::ReadBool(node, wxT("IndentUsesTabs"), m_indentUsesTabs);
        m_indentWidth = XmlUtils::ReadLong(node, wxT("IndentWidth"), m_indentWidth);
        m_tabWidth = XmlUtils::ReadLong(node, wxT("TabWidth"), m_tabWidth);
        m_iconsSize = XmlUtils::ReadLong(node, wxT("ToolbarIconSize"), m_iconsSize);
        m_showWhitespaces = XmlUtils::ReadLong(node, wxT("ShowWhitespaces"), m_showWhitespaces);
        m_foldCompact = XmlUtils::ReadBool(node, wxT("FoldCompact"), m_foldCompact);
        m_foldAtElse = XmlUtils::ReadBool(node, wxT("FoldAtElse"), m_foldAtElse);
        m_foldPreprocessor = XmlUtils::ReadBool(node, wxT("FoldPreprocessor"), m_foldPreprocessor);
        m_highlightMatchedBraces = XmlUtils::ReadBool(node, wxT("HighlightMatchedBraces"), m_highlightMatchedBraces);
        m_foldBgColour = XmlUtils::ReadString(node, wxT("FoldBgColour"), m_foldBgColour.GetAsString(wxC2S_HTML_SYNTAX));
        m_autoAdjustHScrollBarWidth =
            XmlUtils::ReadBool(node, wxT("AutoAdjustHScrollBarWidth"), m_autoAdjustHScrollBarWidth);
        m_caretBlinkPeriod = XmlUtils::ReadLong(node, wxT("CaretBlinkPeriod"), m_caretBlinkPeriod);
        m_caretWidth = XmlUtils::ReadLong(node, wxT("CaretWidth"), m_caretWidth);
        m_copyLineEmptySelection = XmlUtils::ReadBool(node, wxT("CopyLineEmptySelection"), m_copyLineEmptySelection);
        m_smartParen = XmlUtils::ReadBool(node, wxT("SmartParen"), m_smartParen);
        m_showRightMarginIndicator = XmlUtils::ReadBool(node, wxT("ShowRightMargin"), m_showRightMarginIndicator);
        m_rightMarginColumn = XmlUtils::ReadLong(node, wxT("RightMarginnColumn"), m_rightMarginColumn);

        m_programConsoleCommand = XmlUtils::ReadString(node, wxT("ConsoleCommand"), m_programConsoleCommand);
        m_eolMode = XmlUtils::ReadString(node, wxT("EOLMode"), m_eolMode);
        m_trackEditorChanges = XmlUtils::ReadBool(node, wxT("TrackEditorChanges"));
        m_scrollBeyondLastLine = XmlUtils::ReadBool(node, wxT("ScrollBeyondLastLine"), m_scrollBeyondLastLine);
        m_hideOutputPaneOnUserClick = XmlUtils::ReadBool(node, wxT("HideOutputPaneOnUserClick"));
        m_hideOutputPaneNotIfBuild = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfBuild"));
        m_hideOutputPaneNotIfSearch = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfSearch"));
        m_hideOutputPaneNotIfReplace = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfReplace"));
        m_hideOutputPaneNotIfReferences = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfReferences"));
        m_hideOutputPaneNotIfOutput = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfOutput"));
        m_hideOutputPaneNotIfTrace = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfTrace"));
        m_hideOutputPaneNotIfTasks = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfTasks"));
        m_hideOutputPaneNotIfBuildQ = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfBuildQ"));
        m_hideOutputPaneNotIfCppCheck = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfCppCheck"));
        m_hideOutputPaneNotIfSvn = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfSvn"));
        m_hideOutputPaneNotIfCscope = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfCscope"));
        m_hideOutputPaneNotIfGit = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfGit"));
        m_hideOutputPaneNotIfDebug = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfDebug"));
        m_hideOutputPaneNotIfMemCheck = XmlUtils::ReadBool(node, wxT("HideOutputPaneNotIfMemCheck"));
        m_findBarAtBottom = XmlUtils::ReadBool(node, wxT("FindBarAtBottom"), m_findBarAtBottom);
        m_showReplaceBar = XmlUtils::ReadBool(node, wxT("ShowReplaceBar"), m_showReplaceBar);
        m_disableSmartIndent = XmlUtils::ReadBool(node, wxT("DisableSmartIndent"), m_disableSmartIndent);
        m_disableSemicolonShift = XmlUtils::ReadBool(node, wxT("DisableSemicolonShift"), m_disableSemicolonShift);
        m_caretLineAlpha = XmlUtils::ReadLong(node, wxT("CaretLineAlpha"), m_caretLineAlpha);
        m_dontAutoFoldResults = XmlUtils::ReadBool(node, wxT("DontAutoFoldResults"), m_dontAutoFoldResults);
        m_dontOverrideSearchStringWithSelection = XmlUtils::ReadBool(
            node, wxT("DontOverrideSearchStringWithSelection"), m_dontOverrideSearchStringWithSelection);
        m_findNextOrPreviousUseSelection = XmlUtils::ReadBool(
            node, wxT("FindNextOrPreviousUseSelection"), m_findNextOrPreviousUseSelection);
        m_showDebugOnRun = XmlUtils::ReadBool(node, wxT("ShowDebugOnRun"), m_showDebugOnRun);
        m_caretUseCamelCase = XmlUtils::ReadBool(node, wxT("m_caretUseCamelCase"), m_caretUseCamelCase);
        m_wordWrap = XmlUtils::ReadBool(node, wxT("m_wordWrap"), m_wordWrap);
        m_dockingStyle = XmlUtils::ReadLong(node, wxT("m_dockingStyle"), m_dockingStyle);
        m_nbTabHeight = XmlUtils::ReadLong(node, wxT("m_nbTabHeight"), m_nbTabHeight);
        m_mswTheme = XmlUtils::ReadBool(node, wxT("m_mswTheme"), m_mswTheme);
        m_preferredLocale = XmlUtils::ReadString(node, wxT("m_preferredLocale"), m_preferredLocale);
        m_useLocale = XmlUtils::ReadBool(node, wxT("m_useLocale"), m_useLocale);
        m_trimOnlyModifiedLines = XmlUtils::ReadBool(node, wxT("m_trimOnlyModifiedLines"), m_trimOnlyModifiedLines);

        wxString options;
        if (XmlUtils::ReadStringIfExists(node, "options_bits", options)) {
            m_options.from_string(options);
        }
        m_debuggerMarkerLine = XmlUtils::ReadString(
            node, wxT("m_debuggerMarkerLine"), m_debuggerMarkerLine.GetAsString(wxC2S_HTML_SYNTAX));
        m_indentedComments = XmlUtils::ReadBool(node, wxT("IndentedComments"), m_indentedComments);

        // These hacks will likely be changed in the future. If so, we'll be able to remove the #include
        // "editor_config.h" too
        long trim = EditorConfigST::Get()->GetInteger(wxT("EditorTrimEmptyLines"), 0);
        m_TrimLine = (trim > 0);

        long appendLf = EditorConfigST::Get()->GetInteger(wxT("EditorAppendLf"), 0);
        m_AppendLF = (appendLf > 0);

        long dontTrimCaretLine = EditorConfigST::Get()->GetInteger(wxT("DontTrimCaretLine"), 0);
        m_dontTrimCaretLine = (dontTrimCaretLine > 0);

        m_outputTabsDirection =
            (wxDirection)XmlUtils::ReadLong(node, "OutputTabsDirection", (int)m_outputTabsDirection);
        m_workspaceTabsDirection =
            (wxDirection)XmlUtils::ReadLong(node, "SidebarButtonsPosition", (int)m_workspaceTabsDirection);
        m_webSearchPrefix = XmlUtils::ReadString(node, wxT("m_webSearchPrefix"), m_webSearchPrefix);
    }

    // Transitional calls. These checks are relevant for 2 years i.e. until the beginning of 2016
    if (m_bookmarkFgColours.empty()) {
        // This must be the first time with multiple BMs, so rescue any old user-set value
        m_bookmarkFgColours = SetDefaultBookmarkColours();
        wxString oldcolour = XmlUtils::ReadString(node, "BookmarkFgColour", "#FF0080");
        SetBookmarkFgColour(oldcolour, 0);
    }
    if (m_bookmarkBgColours.empty()) {
        m_bookmarkBgColours = SetDefaultBookmarkColours();
        wxString oldcolour = XmlUtils::ReadString(node, "BookmarkBgColour", "#FF0080");
        SetBookmarkBgColour(oldcolour, 0);
    }
}

OptionsConfig::~OptionsConfig() {}

wxXmlNode* OptionsConfig::ToXml() const
{
    wxXmlNode* n = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("Options"));
    n->AddAttribute(wxT("DisplayFoldMargin"), BoolToString(m_displayFoldMargin));
    n->AddAttribute(wxT("UnderlineFoldedLine"), BoolToString(m_underlineFoldLine));
    n->AddAttribute(wxT("FoldStyle"), m_foldStyle);
    n->AddAttribute(wxT("DisplayBookmarkMargin"), BoolToString(m_displayBookmarkMargin));
    n->AddAttribute(wxT("BookmarkShape"), m_bookmarkShape);
    n->AddAttribute(wxT("BookmarkBgColours"), m_bookmarkBgColours);
    n->AddAttribute(wxT("BookmarkFgColours"), m_bookmarkFgColours);
    n->AddAttribute(wxT("BookmarkLabels"), m_bookmarkLabels);
    n->AddAttribute(wxT("ClearHighlitWordsOnFind"), BoolToString(m_clearHighlightedWordsOnFind));
    n->AddAttribute(wxT("HighlightCaretLine"), BoolToString(m_highlightCaretLine));
    n->AddAttribute(wxT("HighlightCaretLineWithColour"), BoolToString(m_highlightCaretLineWithColour));
    n->AddAttribute(wxT("ShowLineNumber"), BoolToString(m_displayLineNumbers));
    n->AddAttribute(wxT("RelativeLineNumber"), BoolToString(m_relativeLineNumbers));
    n->AddAttribute(wxT("LineNumbersHighlightCurrent"), BoolToString(m_lineNumberHighlightCurrent));
    n->AddAttribute(wxT("IndentationGuides"), BoolToString(m_showIndentationGuidelines));
    n->AddAttribute(wxT("CaretLineColour"), m_caretLineColour.GetAsString(wxC2S_HTML_SYNTAX));
    n->AddAttribute(wxT("IndentUsesTabs"), BoolToString(m_indentUsesTabs));
    n->AddAttribute(wxT("FoldCompact"), BoolToString(m_foldCompact));
    n->AddAttribute(wxT("FoldAtElse"), BoolToString(m_foldAtElse));
    n->AddAttribute(wxT("FoldPreprocessor"), BoolToString(m_foldPreprocessor));
    n->AddAttribute(wxT("HighlightMatchedBraces"), BoolToString(m_highlightMatchedBraces));
    n->AddAttribute(wxT("FoldBgColour"), m_foldBgColour.GetAsString(wxC2S_HTML_SYNTAX));
    n->AddAttribute(wxT("AutoAdjustHScrollBarWidth"), BoolToString(m_autoAdjustHScrollBarWidth));
    n->AddAttribute(wxT("TrackEditorChanges"), BoolToString(m_trackEditorChanges));
    n->AddAttribute(wxT("ScrollBeyondLastLine"), BoolToString(m_scrollBeyondLastLine));
    n->AddAttribute(wxT("HideOutputPaneOnUserClick"), BoolToString(m_hideOutputPaneOnUserClick));
    n->AddAttribute(wxT("HideOutputPaneNotIfBuild"), BoolToString(m_hideOutputPaneNotIfBuild));
    n->AddAttribute(wxT("HideOutputPaneNotIfSearch"), BoolToString(m_hideOutputPaneNotIfSearch));
    n->AddAttribute(wxT("HideOutputPaneNotIfReplace"), BoolToString(m_hideOutputPaneNotIfReplace));
    n->AddAttribute(wxT("HideOutputPaneNotIfReferences"), BoolToString(m_hideOutputPaneNotIfReferences));
    n->AddAttribute(wxT("HideOutputPaneNotIfOutput"), BoolToString(m_hideOutputPaneNotIfOutput));
    n->AddAttribute(wxT("HideOutputPaneNotIfTrace"), BoolToString(m_hideOutputPaneNotIfTrace));
    n->AddAttribute(wxT("HideOutputPaneNotIfTasks"), BoolToString(m_hideOutputPaneNotIfTasks));
    n->AddAttribute(wxT("HideOutputPaneNotIfBuildQ"), BoolToString(m_hideOutputPaneNotIfBuildQ));
    n->AddAttribute(wxT("HideOutputPaneNotIfCppCheck"), BoolToString(m_hideOutputPaneNotIfCppCheck));
    n->AddAttribute(wxT("HideOutputPaneNotIfSvn"), BoolToString(m_hideOutputPaneNotIfSvn));
    n->AddAttribute(wxT("HideOutputPaneNotIfCscope"), BoolToString(m_hideOutputPaneNotIfCscope));
    n->AddAttribute(wxT("HideOutputPaneNotIfGit"), BoolToString(m_hideOutputPaneNotIfGit));
    n->AddAttribute(wxT("HideOutputPaneNotIfDebug"), BoolToString(m_hideOutputPaneNotIfDebug));
    n->AddAttribute(wxT("HideOutputPaneNotIfMemCheck"), BoolToString(m_hideOutputPaneNotIfMemCheck));
    n->AddAttribute(wxT("FindBarAtBottom"), BoolToString(m_findBarAtBottom));
    n->AddAttribute(wxT("ShowReplaceBar"), BoolToString(m_showReplaceBar));
    n->AddAttribute(wxT("DisableSmartIndent"), BoolToString(m_disableSmartIndent));
    n->AddAttribute(wxT("DisableSemicolonShift"), BoolToString(m_disableSemicolonShift));
    n->AddAttribute(wxT("DontAutoFoldResults"), BoolToString(m_dontAutoFoldResults));
    n->AddAttribute(wxT("DontOverrideSearchStringWithSelection"),
                    BoolToString(m_dontOverrideSearchStringWithSelection));
    n->AddAttribute(wxT("FindNextOrPreviousUseSelection"),
                    BoolToString(m_findNextOrPreviousUseSelection));
    n->AddAttribute(wxT("ShowDebugOnRun"), BoolToString(m_showDebugOnRun));
    n->AddAttribute(wxT("ConsoleCommand"), m_programConsoleCommand);
    n->AddAttribute(wxT("EOLMode"), m_eolMode);
    n->AddAttribute(wxT("m_caretUseCamelCase"), BoolToString(m_caretUseCamelCase));
    n->AddAttribute(wxT("m_wordWrap"), BoolToString(m_wordWrap));
    n->AddAttribute(wxT("m_dockingStyle"), wxString::Format(wxT("%d"), m_dockingStyle));
    n->AddAttribute(wxT("m_nbTabHeight"), wxString::Format(wxT("%d"), m_nbTabHeight));
    n->AddAttribute(wxT("m_mswTheme"), BoolToString(m_mswTheme));
    n->AddAttribute(wxT("m_preferredLocale"), m_preferredLocale);
    n->AddAttribute(wxT("m_useLocale"), BoolToString(m_useLocale));
    n->AddAttribute(wxT("m_trimOnlyModifiedLines"), BoolToString(m_trimOnlyModifiedLines));
    n->AddAttribute(wxT("m_debuggerMarkerLine"), m_debuggerMarkerLine.GetAsString(wxC2S_HTML_SYNTAX));
    n->AddAttribute(wxT("OutputTabsDirection"), wxString() << (int)m_outputTabsDirection);
    n->AddAttribute(wxT("SidebarButtonsPosition"), wxString() << (int)m_workspaceTabsDirection);
    n->AddAttribute(wxT("IndentedComments"), BoolToString(m_indentedComments));
    n->AddAttribute(wxT("CopyLineEmptySelection"), BoolToString(m_copyLineEmptySelection));
    n->AddAttribute(wxT("SmartParen"), BoolToString(m_smartParen));
    n->AddAttribute(wxT("ShowRightMargin"), BoolToString(m_showRightMarginIndicator));
    n->AddAttribute(wxT("RightMarginnColumn"), wxString() << m_rightMarginColumn);

    wxString tmp;
    tmp << m_indentWidth;
    n->AddAttribute(wxT("IndentWidth"), tmp);

    tmp.clear();
    tmp << m_tabWidth;
    n->AddAttribute(wxT("TabWidth"), tmp);

    tmp.clear();
    tmp << m_iconsSize;
    n->AddAttribute(wxT("ToolbarIconSize"), tmp);

    tmp.clear();
    tmp << m_showWhitespaces;
    n->AddAttribute(wxT("ShowWhitespaces"), tmp);

    tmp.clear();
    tmp << m_caretWidth;
    n->AddAttribute(wxT("CaretWidth"), tmp);

    tmp.clear();
    tmp << m_caretBlinkPeriod;
    n->AddAttribute(wxT("CaretBlinkPeriod"), tmp);

    tmp.clear();
    tmp << m_caretLineAlpha;
    n->AddAttribute(wxT("CaretLineAlpha"), tmp);

    tmp.clear();
    tmp = wxFontMapper::GetEncodingName(m_fileFontEncoding);
    n->AddAttribute(wxT("FileFontEncoding"), tmp);

    n->AddAttribute(wxT("options_bits"), m_options.to_string());
    n->AddAttribute(wxT("m_webSearchPrefix"), m_webSearchPrefix);
    return n;
}

void OptionsConfig::SetFileFontEncoding(const wxString& strFileFontEncoding)
{
    this->m_fileFontEncoding = wxFontMapper::Get()->CharsetToEncoding(strFileFontEncoding, false);

    if (wxFONTENCODING_SYSTEM == this->m_fileFontEncoding) {
        this->m_fileFontEncoding = wxFONTENCODING_UTF8;
    }
}

wxString OptionsConfig::GetEOLAsString() const
{
    if (GetEolMode() == wxT("Unix (LF)")) {
        return "\n";

    } else if (GetEolMode() == wxT("Mac (CR)")) {
        return "\r";

    } else if (GetEolMode() == wxT("Windows (CRLF)")) {
        return "\r\n";

    } else {
        return "\n";
    }
}

wxColour OptionsConfig::GetBookmarkFgColour(size_t index) const
{
    wxColour col;
    wxArrayString arr = wxSplit(m_bookmarkFgColours, ';');
    if (index < arr.GetCount()) {
        return wxColour(arr.Item(index));
    }

    return col;
}

void OptionsConfig::SetBookmarkFgColour(wxColour c, size_t index)
{
    wxArrayString arr = wxSplit(m_bookmarkFgColours, ';');
    if (index < arr.GetCount()) {
        arr.Item(index) = c.GetAsString(wxC2S_HTML_SYNTAX);
        m_bookmarkFgColours = wxJoin(arr, ';');
    }
}

wxColour OptionsConfig::GetBookmarkBgColour(size_t index) const
{
    wxColour col;
    wxArrayString arr = wxSplit(m_bookmarkBgColours, ';');
    if (index < arr.GetCount()) {
        return wxColour(arr.Item(index));
    }

    return col;
}

void OptionsConfig::SetBookmarkBgColour(wxColour c, size_t index)
{
    wxArrayString arr = wxSplit(m_bookmarkBgColours, ';');
    if (index < arr.GetCount()) {
        arr.Item(index) = c.GetAsString(wxC2S_HTML_SYNTAX);
        m_bookmarkBgColours = wxJoin(arr, ';');
    }
}

wxString OptionsConfig::GetBookmarkLabel(size_t index) const
{
    wxArrayString arr = wxSplit(m_bookmarkLabels, ';');
    if (index < arr.GetCount()) {
        return arr.Item(index);
    }

    return "";
}

void OptionsConfig::SetBookmarkLabel(const wxString& label, size_t index)
{
    wxArrayString arr = wxSplit(m_bookmarkLabels, ';');
    if (index < arr.GetCount()) {
        arr.Item(index) = label;
        m_bookmarkLabels = wxJoin(arr, ';');
    }
}

void OptionsConfig::UpdateFromEditorConfig(const clEditorConfigSection& section)
{
    if (section.IsInsertFinalNewlineSet()) {
        this->SetAppendLF(section.IsInsertFinalNewline());
    }
    if (section.IsSetEndOfLineSet()) {
        // Convert .editorconfig to CodeLite strings
        wxString eolMode = "Unix (LF)"; // default
        if (section.GetEndOfLine() == "crlf") {
            eolMode = "Windows (CRLF)";
        } else if (section.GetEndOfLine() == "cr") {
            eolMode = "Mac (CR)";
        }
        this->SetEolMode(eolMode);
    }
    if (section.IsTabWidthSet()) {
        this->SetTabWidth(section.GetTabWidth());
    }
    if (section.IsIndentStyleSet()) {
        this->SetIndentUsesTabs(section.GetIndentStyle() == "tab");
    }
    if (section.IsTabWidthSet()) {
        this->SetTabWidth(section.GetTabWidth());
    }
    if (section.IsIndentSizeSet()) {
        this->SetIndentWidth(section.GetIndentSize());
    }
    if (section.IsCharsetSet()) {
        // TODO: fix the locale here
    }
}

bool OptionsConfig::IsTabColourDark() const { return HasOption(Opt_TabColourDark); }
bool OptionsConfig::IsTabColourMatchesTheme() const { return !HasOption(Opt_TabColourPersistent); }
void OptionsConfig::EnableOption(size_t flag, bool b) { m_options.set(flag, b); }
bool OptionsConfig::HasOption(size_t flag) const { return m_options.test(flag); }
