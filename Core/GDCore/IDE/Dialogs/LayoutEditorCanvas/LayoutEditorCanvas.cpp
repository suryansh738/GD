/** \file
 *  Game Develop
 *  2008-2013 Florian Rival (Florian.Rival@gmail.com)
 */
#include "LayoutEditorCanvas.h"
#include <cmath>
#include <wx/wx.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/page.h>
#include <wx/ribbon/buttonbar.h>
#include <wx/aui/aui.h>
#include <SFML/Graphics.hpp>
#include "GDCore/PlatformDefinition/LayoutEditorPreviewer.h"
#include "GDCore/PlatformDefinition/Platform.h"
#include "GDCore/PlatformDefinition/Project.h"
#include "GDCore/PlatformDefinition/Layout.h"
#include "GDCore/PlatformDefinition/Object.h"
#include "GDCore/PlatformDefinition/InitialInstance.h"
#include "GDCore/PlatformDefinition/InitialInstancesContainer.h"
#include "GDCore/IDE/Dialogs/LayoutEditorCanvas/LayoutEditorCanvasAssociatedEditor.h"
#include "GDCore/IDE/Dialogs/LayoutEditorCanvas/LayoutEditorCanvasTextDnd.h"
#include "GDCore/IDE/Dialogs/LayoutEditorCanvas/LayoutEditorCanvasOptions.h"
#include "GDCore/IDE/Dialogs/MainFrameWrapper.h"
#include "GDCore/IDE/Dialogs/GridSetupDialog.h"
#include "GDCore/IDE/CommonBitmapManager.h"
#include "GDCore/CommonTools.h"
// Platform-specific includes. Be sure to include them at the end as it seems to be some incompatibilities with SFML's WindowStyle.hpp
#ifdef __WXGTK__
    #include <gdk/gdkx.h>
    #include <gtk/gtk.h>
    #include <wx/gtk/private/win_gtk.h> //If this file is unable during compilation, then you must manually locate the "gtk/private" folder it in the wxWidgets folder and copy it into the folder where wx is installed.
#endif

//(*InternalHeaders(LayoutEditorCanvas)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(LayoutEditorCanvas)
//*)

using namespace std;

namespace gd
{

BEGIN_EVENT_TABLE(LayoutEditorCanvas,wxPanel)
	//(*EventTable(LayoutEditorCanvas)
	//*)
END_EVENT_TABLE()

const long LayoutEditorCanvas::idRibbonEditMode = wxNewId();
const long LayoutEditorCanvas::idRibbonPreviewMode = wxNewId();
const long LayoutEditorCanvas::idRibbonHelp = wxNewId();
const long LayoutEditorCanvas::idRibbonObjectsEditor = wxNewId();
const long LayoutEditorCanvas::idRibbonLayersEditor = wxNewId();
const long LayoutEditorCanvas::idRibbonGrid = wxNewId();
const long LayoutEditorCanvas::idRibbonWindowMask = wxNewId();
const long LayoutEditorCanvas::idRibbonGridSetup = wxNewId();
const long LayoutEditorCanvas::idRibbonUndo = wxNewId();
const long LayoutEditorCanvas::idRibbonRedo = wxNewId();
const long LayoutEditorCanvas::idRibbonObjectsPositionList = wxNewId();
const long LayoutEditorCanvas::idUndo10 = wxNewId();
const long LayoutEditorCanvas::idUndo20 = wxNewId();
const long LayoutEditorCanvas::idClearHistory = wxNewId();
const long LayoutEditorCanvas::idRibbonFullScreen = wxNewId();
const long LayoutEditorCanvas::ID_ADDOBJMENU = wxNewId();
const long LayoutEditorCanvas::ID_DELOBJMENU = wxNewId();
const long LayoutEditorCanvas::ID_PROPMENU = wxNewId();
const long LayoutEditorCanvas::ID_LAYERUPMENU = wxNewId();
const long LayoutEditorCanvas::ID_LAYERDOWNMENU = wxNewId();
const long LayoutEditorCanvas::ID_COPYMENU = wxNewId();
const long LayoutEditorCanvas::ID_CUTMENU = wxNewId();
const long LayoutEditorCanvas::ID_PASTEMENU = wxNewId();
const long LayoutEditorCanvas::ID_PASTESPECIALMENU = wxNewId();
const long LayoutEditorCanvas::ID_CREATEOBJECTMENU = wxNewId();
const long LayoutEditorCanvas::ID_LOCKMENU = wxNewId();
const long LayoutEditorCanvas::ID_UNLOCKMENU = wxNewId();
const long LayoutEditorCanvas::idRibbonOrigine = wxNewId();
const long LayoutEditorCanvas::idRibbonOriginalZoom = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM500 = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM200 = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM150 = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM100 = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM50 = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM25 = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM10 = wxNewId();
const long LayoutEditorCanvas::ID_CUSTOMZOOMMENUITEM5 = wxNewId();

LayoutEditorCanvas::LayoutEditorCanvas(wxWindow* parent, gd::Project & project_, gd::Layout & layout_, gd::InitialInstancesContainer & instances_, LayoutEditorCanvasOptions & options_, gd::MainFrameWrapper & mainFrameWrapper_) :
    project(project_),
    layout(layout_),
    instances(instances_),
    options(options_),
    mainFrameWrapper(mainFrameWrapper_),
    parentControl(parent),
    parentAuiManager(NULL),
    hScrollbar(NULL),
    vScrollbar(NULL),
    isMovingView(false),
    hasJustRightClicked(false),
    ctrlPressed(false),
    altPressed(false),
    shiftPressed(false),
    isMovingInstance(false),
    isSelecting(false),
    editing(true)
{
	//(*Initialize(LayoutEditorCanvas)
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("wxID_ANY"));
	//*)

    //Initialization allowing to run SFML within the wxWidgets control.
    //See also LayoutEditorCanvas::OnUpdate & co.
    #ifdef __WXGTK__

        // GTK implementation requires to go deeper to find the low-level X11 identifier of the widget
        gtk_widget_realize(m_wxwindow);
        gtk_widget_set_double_buffered(m_wxwindow, false);

        GtkWidget* privHandle = m_wxwindow;
        wxPizza * pizza = WX_PIZZA(privHandle);
        GtkWidget * widget = GTK_WIDGET(pizza);

        GdkWindow* Win = widget->window;
        XFlush(GDK_WINDOW_XDISPLAY(Win));
        sf::RenderWindow::create(GDK_WINDOW_XWINDOW(Win));

    #else

        // Tested under Windows XP only (should work with X11 and other Windows versions - no idea about MacOS)
        sf::RenderWindow::create(static_cast<sf::WindowHandle>(GetHandle()));

    #endif

	Connect(wxEVT_PAINT,(wxObjectEventFunction)&LayoutEditorCanvas::OnPaint);
	Connect(wxEVT_ERASE_BACKGROUND,(wxObjectEventFunction)&LayoutEditorCanvas::OnEraseBackground);
	Connect(wxEVT_IDLE,(wxObjectEventFunction)&LayoutEditorCanvas::OnIdle);
	Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&LayoutEditorCanvas::OnLeftDown);
	Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&LayoutEditorCanvas::OnLeftUp);
	Connect(wxEVT_LEFT_DCLICK,(wxObjectEventFunction)&LayoutEditorCanvas::OnLeftDClick);
	Connect(wxEVT_RIGHT_UP,(wxObjectEventFunction)&LayoutEditorCanvas::OnRightUp);
	Connect(wxEVT_MIDDLE_DOWN,(wxObjectEventFunction)&LayoutEditorCanvas::OnMiddleDown);
	Connect(wxEVT_MOTION,(wxObjectEventFunction)&LayoutEditorCanvas::OnMotion);
	Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&LayoutEditorCanvas::OnKey);
	Connect(wxEVT_KEY_UP,(wxObjectEventFunction)&LayoutEditorCanvas::OnKeyUp);
	Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&LayoutEditorCanvas::OnMouseWheel);
    Connect(ID_DELOBJMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnDeleteObjectSelected);
    Connect(ID_PROPMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnPropObjSelected);
    Connect(ID_LAYERUPMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnLayerUpSelected);
    Connect(ID_LAYERDOWNMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnLayerDownSelected);
    Connect(ID_COPYMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCopySelected);
    Connect(ID_CUTMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCutSelected);
    Connect(ID_PASTEMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnPasteSelected);
    Connect(ID_PASTESPECIALMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnPasteSpecialSelected);
    Connect(ID_CREATEOBJECTMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCreateObjectSelected);
    Connect(ID_LOCKMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnLockSelected);
    Connect(ID_UNLOCKMENU,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnUnLockSelected);
    SetDropTarget(new LayoutEditorCanvasTextDnd(*this));

    //Generate undo menu
    {
        wxMenuItem * undo10item = new wxMenuItem(&undoMenu, idUndo10, _("Cancel 10 changes"), wxEmptyString, wxITEM_NORMAL);
        undoMenu.Append(undo10item);
        wxMenuItem * undo20item = new wxMenuItem(&undoMenu, idUndo20, _("Cancel 20 changes"), wxEmptyString, wxITEM_NORMAL);
        undoMenu.Append(undo20item);
        undoMenu.AppendSeparator();
        wxMenuItem * clearHistoryItem = new wxMenuItem(&undoMenu, idClearHistory, _("Delete the historic"), wxEmptyString, wxITEM_NORMAL);
        clearHistoryItem->SetBitmap(wxImage( "res/history_clear16.png" ) );
        undoMenu.Append(clearHistoryItem);
    }

    //Prepare undo-related variables
    latestState = boost::shared_ptr<gd::InitialInstancesContainer>(instances.Clone());

    //Generate zoom menu
	wxMenuItem * zoom5 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM5, _("5%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom5);
	wxMenuItem * zoom10 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM10, _("10%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom10);
	wxMenuItem * zoom25 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM25, _("25%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom25);
	wxMenuItem * zoom50 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM50, _("50%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom50);
	wxMenuItem * zoom100 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM100, _("100%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom100);
	wxMenuItem * zoom150 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM150, _("150%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom150);
	wxMenuItem * zoom200 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM200, _("200%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom200);
	wxMenuItem * zoom500 = new wxMenuItem((&zoomMenu), ID_CUSTOMZOOMMENUITEM500, _("500%"), wxEmptyString, wxITEM_NORMAL);
	zoomMenu.Append(zoom500);

    //Generate context menu
    {
        wxMenuItem * layerUpItem = new wxMenuItem((&contextMenu), ID_LAYERUPMENU, _("Put the object(s) on the higher layer"), wxEmptyString, wxITEM_NORMAL);
        layerUpItem->SetBitmap(wxImage( "res/up.png" ) );
        wxMenuItem * layerDownItem = new wxMenuItem((&contextMenu), ID_LAYERDOWNMENU, _("Put the object(s) on the lower layer"), wxEmptyString, wxITEM_NORMAL);
        layerDownItem->SetBitmap(wxImage( "res/down.png" ) );
        wxMenuItem * deleteItem = new wxMenuItem((&contextMenu), ID_DELOBJMENU, _("Delete the selection\tDEL"), wxEmptyString, wxITEM_NORMAL);
        deleteItem->SetBitmap(wxImage( "res/deleteicon.png" ) );

        contextMenu.Append(ID_PROPMENU, _("Properties"));
        contextMenu.AppendSeparator();
        contextMenu.Append(ID_CREATEOBJECTMENU, _("Insert a new object"));
        contextMenu.AppendSeparator();
        contextMenu.Append(deleteItem);
        contextMenu.AppendSeparator();
        contextMenu.Append(layerUpItem);
        contextMenu.Append(layerDownItem);
        contextMenu.AppendSeparator();

        wxMenuItem * copyItem = new wxMenuItem((&contextMenu), ID_COPYMENU, _("Copy"), wxEmptyString, wxITEM_NORMAL);
        copyItem->SetBitmap(wxImage( "res/copyicon.png" ) );
        contextMenu.Append(copyItem);
        wxMenuItem * cutItem = new wxMenuItem((&contextMenu), ID_CUTMENU, _("Cut"), wxEmptyString, wxITEM_NORMAL);
        cutItem->SetBitmap(wxImage( "res/cuticon.png" ) );
        contextMenu.Append(cutItem);
        wxMenuItem * pasteItem = new wxMenuItem((&contextMenu), ID_PASTEMENU, _("Paste"), wxEmptyString, wxITEM_NORMAL);
        pasteItem->SetBitmap(wxImage( "res/pasteicon.png" ) );
        contextMenu.Append(pasteItem);
        wxMenuItem * pasteSpecialItem = new wxMenuItem((&contextMenu), ID_PASTESPECIALMENU, _("Special paste"), wxEmptyString, wxITEM_NORMAL);
        contextMenu.Append(pasteSpecialItem);

        contextMenu.AppendSeparator();
        wxMenuItem * lockItem = new wxMenuItem((&contextMenu), ID_LOCKMENU, _("Lock the object(s)"), wxEmptyString, wxITEM_NORMAL);
        lockItem->SetBitmap(wxImage( "res/lockicon.png" ) );
        contextMenu.Append(lockItem);
    }

    //Generate "no object" context menu
    {
        noObjectContextMenu.Append(ID_CREATEOBJECTMENU, _("Insert a new object"));
        noObjectContextMenu.AppendSeparator();
        wxMenuItem * pasteItem = new wxMenuItem((&noObjectContextMenu), ID_PASTEMENU, _("Paste"), wxEmptyString, wxITEM_NORMAL);
        pasteItem->SetBitmap(wxImage( "res/pasteicon.png" ) );
        noObjectContextMenu.Append(pasteItem);
        wxMenuItem * pasteSpecialItem = new wxMenuItem((&noObjectContextMenu), ID_PASTESPECIALMENU, _("Special paste"), wxEmptyString, wxITEM_NORMAL);
        noObjectContextMenu.Append(pasteSpecialItem);
        noObjectContextMenu.AppendSeparator();
        wxMenuItem * unlockItem = new wxMenuItem((&noObjectContextMenu), ID_UNLOCKMENU, _("Unlock the object under the cursor"), wxEmptyString, wxITEM_NORMAL);
        unlockItem->SetBitmap(wxImage( "res/lockicon.png" ) );
        noObjectContextMenu.Append(unlockItem);
    }

    //Initialize previewers
    for (unsigned int i = 0;i<project.GetUsedPlatforms().size();++i)
    {
        boost::shared_ptr<gd::LayoutEditorPreviewer> previewer = project.GetUsedPlatforms()[i]->GetLayoutPreviewer(*this);
        previewers[project.GetUsedPlatforms()[i]->GetName()] = previewer;
        if ( i == 0 ) currentPreviewer = previewer;

        long id = wxNewId();
        if ( previewer ) {
            idForPlatformsMenu[id] = project.GetUsedPlatforms()[i]->GetName();
            platformsMenu.Append(id, _("Preview for ") + project.GetUsedPlatforms()[i]->GetFullName(),
                                 _("Launch a preview for this platform"), wxITEM_RADIO);
            mainFrameWrapper.GetMainEditor()->Connect(id, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutEditorCanvas::OnPreviewForPlatformSelected, NULL, this);
        }
        else {
            platformsMenu.Append(id, _("No preview available for ")+ project.GetUsedPlatforms()[i]->GetFullName(), _("No preview can be done for this platform"), wxITEM_RADIO);
            platformsMenu.Enable(id, false);
        }
    }

    setFramerateLimit(30);
    editionView.setCenter( (project.GetMainWindowDefaultWidth()/2),(project.GetMainWindowDefaultHeight()/2));
    RecreateRibbonToolbar();
    ReloadResources();
}

LayoutEditorCanvas::~LayoutEditorCanvas()
{
	//(*Destroy(LayoutEditorCanvas)
	//*)
}

void LayoutEditorCanvas::OnIdle(wxIdleEvent&)
{
    // Send a paint message when the control is idle, to ensure maximum framerate
    Refresh();
}

void LayoutEditorCanvas::OnPaint(wxPaintEvent&)
{
    // Make sure the control is able to be repainted
    wxPaintDC Dc(this);
    OnUpdate();
}

void LayoutEditorCanvas::OnEraseBackground(wxEraseEvent&) {}

void LayoutEditorCanvas::AddAssociatedEditor(gd::LayoutEditorCanvasAssociatedEditor * editor)
{
    if (!editor) return;

    associatedEditors.insert(editor);
}

void LayoutEditorCanvas::ConnectEvents()
{
    if (!editing && currentPreviewer) currentPreviewer->ConnectPreviewRibbonToolsEvents();

    mainFrameWrapper.GetMainEditor()->Connect(idRibbonEditMode, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnEditionBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonPreviewMode, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnPreviewBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonPreviewMode, wxEVT_COMMAND_RIBBONBUTTON_DROPDOWN_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnPreviewDropDownBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonHelp,wxEVT_COMMAND_RIBBONBUTTON_CLICKED,(wxObjectEventFunction)&LayoutEditorCanvas::OnHelpBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonObjectsEditor, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnObjectsEditor, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonLayersEditor, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnLayersEditor, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonGrid, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnGridBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonGridSetup, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnGridSetupBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonWindowMask, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnWindowMaskBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonUndo,wxEVT_COMMAND_RIBBONBUTTON_CLICKED,(wxObjectEventFunction)&LayoutEditorCanvas::OnUndoBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonUndo, wxEVT_COMMAND_RIBBONBUTTON_DROPDOWN_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnUndoMoreBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonRedo,wxEVT_COMMAND_RIBBONBUTTON_CLICKED,(wxObjectEventFunction)&LayoutEditorCanvas::OnRedoBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonObjectsPositionList,wxEVT_COMMAND_RIBBONBUTTON_CLICKED,(wxObjectEventFunction)&LayoutEditorCanvas::OnObjectsPositionList, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idUndo10,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnUndo10Selected, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idUndo20,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnUndo20Selected, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idClearHistory,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnClearHistorySelected, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonFullScreen,wxEVT_COMMAND_RIBBONBUTTON_CLICKED,(wxObjectEventFunction)&LayoutEditorCanvas::OnFullScreenBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonOrigine, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnOrigineBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonOriginalZoom, wxEVT_COMMAND_RIBBONBUTTON_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnZoomInitBtClick, NULL, this);
    mainFrameWrapper.GetMainEditor()->Connect(idRibbonOriginalZoom, wxEVT_COMMAND_RIBBONBUTTON_DROPDOWN_CLICKED, (wxObjectEventFunction)&LayoutEditorCanvas::OnZoomMoreBtClick, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom5Selected, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM10,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom10Selected, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM25,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom25Selected, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM50,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom50Selected, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM100,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom100Selected, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM150,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom150Selected, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM200,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom200Selected, NULL, this);
	mainFrameWrapper.GetMainEditor()->Connect(ID_CUSTOMZOOMMENUITEM500,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&LayoutEditorCanvas::OnCustomZoom500Selected, NULL, this);
}

void LayoutEditorCanvas::OnGuiElementHovered(const gd::LayoutEditorCanvasGuiElement & guiElement)
{
    UpdateMouseResizeCursor(guiElement.name);
}

void LayoutEditorCanvas::OnGuiElementPressed(const gd::LayoutEditorCanvasGuiElement & guiElement)
{
    if ( currentDraggableBt.empty() && guiElement.name.substr(0, 6) == "resize" )
    {
        currentDraggableBt = guiElement.name;

        resizeOriginalWidths.clear();
        resizeOriginalHeights.clear();
        for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        {
            it->second.x = it->first->GetX(); it->second.y = it->first->GetY();

            if ( it->first->HasCustomSize() ) {
                resizeOriginalWidths[it->first] = it->first->GetCustomWidth();
                resizeOriginalHeights[it->first] = it->first->GetCustomHeight();
            }
            else {
                gd::Object * associatedObject = GetObjectLinkedToInitialInstance(*(it->first));
                if ( associatedObject )
                {
                    sf::Vector2f size = associatedObject->GetInitialInstanceDefaultSize(*(it->first), project, layout);
                    resizeOriginalWidths[it->first] = size.x;
                    resizeOriginalHeights[it->first] = size.y;
                }
            }
        }
        resizeMouseStartPosition = sf::Vector2f(GetMouseXOnLayout(), GetMouseYOnLayout());
    }
    else if ( currentDraggableBt.empty() && guiElement.name == "angle" )
    {
        currentDraggableBt = "angle";
    }
}

void LayoutEditorCanvas::OnGuiElementReleased(const gd::LayoutEditorCanvasGuiElement & guiElement)
{
}

void LayoutEditorCanvas::OnPreviewForPlatformSelected( wxCommandEvent & event )
{
    std::string platformName = idForPlatformsMenu[event.GetId()];
    currentPreviewer = previewers[platformName];

    wxCommandEvent useless;
    OnPreviewBtClick(useless);
}

/**
 * Go in preview mode
 */
void LayoutEditorCanvas::OnPreviewBtClick( wxCommandEvent & event )
{
    if ( !editing ) return;

    if ( !currentPreviewer ) {
        wxLogMessage(_("This platform does not support launching previews."));
        return;
    }

    editing = false;

    if (!currentPreviewer->LaunchPreview())
    {
        //Do not go into preview state if LaunchPreview returned false
        //( Some platforms can launch a program and do not display the preview in the editor. )
        editing = true;
        wxSetWorkingDirectory(mainFrameWrapper.GetIDEWorkingDirectory());
        return;
    }

    std::cout << "Switching to preview mode..." << std::endl;

    UpdateSize();
    UpdateScrollbars();

    //Let the IDE go into to preview state
    //Note: Working directory is changed later, just before loading the layout
    mainFrameWrapper.LockShortcuts(this);
    mainFrameWrapper.DisableControlsForScenePreviewing();
    for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->Enable(false);

    wxSetWorkingDirectory(mainFrameWrapper.GetIDEWorkingDirectory());
    RecreateRibbonToolbar();
    currentPreviewer->ConnectPreviewRibbonToolsEvents();
    hScrollbar->Show(false);
    vScrollbar->Show(false);
    SetFocus();

    if ( wxDirExists(wxFileName::FileName(project.GetProjectFile()).GetPath()))
        wxSetWorkingDirectory(wxFileName::FileName(project.GetProjectFile()).GetPath());
}

/**
 * Go in edition mode
 */
void LayoutEditorCanvas::OnEditionBtClick( wxCommandEvent & event )
{
    if ( editing ) return;
    std::cout << "Switching to edition mode..." << std::endl;
    editing = true;

    //Notice the previewer it must stop preview.
    if ( currentPreviewer ) currentPreviewer->StopPreview();

    //Let the IDE go back to edition state
    UpdateSize();
    UpdateScrollbars();
    ReloadResources();
    wxSetWorkingDirectory(mainFrameWrapper.GetIDEWorkingDirectory());

    mainFrameWrapper.UnLockShortcuts(this);
    mainFrameWrapper.EnableControlsAfterScenePreviewing();
    for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->Enable();

    RecreateRibbonToolbar();
    hScrollbar->Show(true);
    vScrollbar->Show(true);
}

wxRibbonButtonBar* LayoutEditorCanvas::CreateRibbonPage(wxRibbonPage * page)
{
    bool hideLabels = false;
    wxConfigBase::Get()->Read( _T( "/Skin/HideLabels" ), &hideLabels );

    {
        wxRibbonPanel *ribbonPanel = new wxRibbonPanel(page, wxID_ANY, _("Mode"), wxBitmap("res/preview24.png", wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_DEFAULT_STYLE);
        wxRibbonButtonBar *ribbonBar = new wxRibbonButtonBar(ribbonPanel, wxID_ANY);
        ribbonBar->AddButton(idRibbonEditMode, !hideLabels ? _("Edition") : "", wxBitmap("res/edit24.png", wxBITMAP_TYPE_ANY));
        ribbonBar->AddButton(idRibbonPreviewMode, !hideLabels ? _("Preview") : "", wxBitmap("res/preview24.png", wxBITMAP_TYPE_ANY), _("Launch a preview of the layout"), wxRIBBON_BUTTON_HYBRID);
    }

    wxRibbonPanel *toolsPanel = new wxRibbonPanel(page, wxID_ANY, _("Tools"), wxBitmap("res/tools24.png", wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_DEFAULT_STYLE);
    wxRibbonButtonBar * ribbonToolbar = new wxRibbonButtonBar(toolsPanel, wxID_ANY);

    {
        wxRibbonPanel *ribbonPanel = new wxRibbonPanel(page, wxID_ANY, _("Help"), wxBitmap("res/helpicon24.png", wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_DEFAULT_STYLE);
        wxRibbonButtonBar *ribbonBar = new wxRibbonButtonBar(ribbonPanel, wxID_ANY);
        ribbonBar->AddButton(idRibbonHelp, !hideLabels ? _("Help") : "", wxBitmap("res/helpicon24.png", wxBITMAP_TYPE_ANY));
    }

    return ribbonToolbar;
}

void LayoutEditorCanvas::RecreateRibbonToolbar()
{
    mainFrameWrapper.GetRibbonSceneEditorButtonBar()->ClearButtons();

    if ( editing )
        CreateEditionRibbonTools();
    else
    {
        if (currentPreviewer) currentPreviewer->CreatePreviewRibbonTools(*mainFrameWrapper.GetRibbonSceneEditorButtonBar());
    }

    mainFrameWrapper.GetRibbonSceneEditorButtonBar()->Realize();
}

void LayoutEditorCanvas::CreateEditionRibbonTools()
{
    bool hideLabels = false;
    wxConfigBase::Get()->Read( _T( "/Skin/HideLabels" ), &hideLabels );
    gd::CommonBitmapManager * bitmapManager = gd::CommonBitmapManager::GetInstance();

    wxRibbonButtonBar * ribbonToolbar = mainFrameWrapper.GetRibbonSceneEditorButtonBar();
    ribbonToolbar->AddButton(idRibbonObjectsEditor, !hideLabels ? _("Objects") : "", bitmapManager->objects24);
    ribbonToolbar->AddButton(idRibbonLayersEditor, !hideLabels ? _("Layers editor") : "", bitmapManager->layers24);
    ribbonToolbar->AddButton(idRibbonObjectsPositionList, !hideLabels ? _("Instances") : "", bitmapManager->objectsPositionsList24);
    ribbonToolbar->AddHybridButton(idRibbonUndo, !hideLabels ? _("Cancel") : "", bitmapManager->undo24);
    ribbonToolbar->AddButton(idRibbonRedo, !hideLabels ? _("Redo") : "", bitmapManager->redo24);
    ribbonToolbar->AddButton(idRibbonGrid, !hideLabels ? _("Grid") : "", bitmapManager->grid24);
    ribbonToolbar->AddButton(idRibbonGridSetup, !hideLabels ? _("Edit the grid") : "", bitmapManager->gridedit24);
    ribbonToolbar->AddButton(idRibbonWindowMask, !hideLabels ? _("Mask") : "", bitmapManager->windowMask24);
    ribbonToolbar->AddButton(idRibbonOrigine, !hideLabels ? _("Return to the initial position ( 0;0 )") : "", bitmapManager->center24);
    ribbonToolbar->AddHybridButton(idRibbonOriginalZoom, !hideLabels ? _("Initial zoom") : "", bitmapManager->zoom24);
}

void LayoutEditorCanvas::UpdateContextMenu()
{
    if ( selectedInstances.empty() ) return;

    //Can we send the objects on a higher layer ?
    unsigned int lowestLayer = layout.GetLayersCount()-1;
    for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
    {
        if (it->first == NULL) continue;
        lowestLayer = std::min(lowestLayer, layout.GetLayerPosition(it->first->GetLayer()));
    }

    contextMenu.FindItem(ID_LAYERUPMENU)->Enable(false);
    if ( lowestLayer+1 < layout.GetLayersCount() )
    {
        string name = layout.GetLayer(lowestLayer+1).GetName();
        if ( name == "" ) name = _("Base layer");
        contextMenu.FindItem(ID_LAYERUPMENU)->Enable(true);
        contextMenu.FindItem(ID_LAYERUPMENU)->SetItemLabel(string(_("Put the object(s) on the layer \"")) + name +"\"");
    }

    //Can we send the objects on a lower layer ?
    unsigned int highestLayer = 0;
    for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
    {
        if (it->first == NULL) continue;
        highestLayer = std::max(highestLayer, layout.GetLayerPosition(it->first->GetLayer()));
    }

    contextMenu.FindItem(ID_LAYERDOWNMENU)->Enable(false);
    if ( highestLayer >= 1 )
    {
        string name = layout.GetLayer(highestLayer-1).GetName();
        if ( name == "" ) name = _("Base layer");

        contextMenu.FindItem(ID_LAYERDOWNMENU)->Enable(true);
        contextMenu.FindItem(ID_LAYERDOWNMENU)->SetItemLabel(string(_("Put the object(s) on the layer \"")) + name +"\"");
    }
}

void LayoutEditorCanvas::OnLayerUpSelected(wxCommandEvent & event)
{
    unsigned int lowestLayer = layout.GetLayersCount()-1;
    for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
    {
        if (it->first == NULL) continue;
        lowestLayer = std::min(lowestLayer, layout.GetLayerPosition(it->first->GetLayer()));
    }

    if ( lowestLayer+1 < layout.GetLayersCount() ) SendSelectionToLayer(layout.GetLayer(lowestLayer+1).GetName());
}

void LayoutEditorCanvas::OnLayerDownSelected(wxCommandEvent & event)
{
    unsigned int highestLayer = 0;
    for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
    {
        if (it->first == NULL) continue;
        highestLayer = std::max(highestLayer, layout.GetLayerPosition(it->first->GetLayer()));
    }

    if ( highestLayer >= 1 ) SendSelectionToLayer(layout.GetLayer(highestLayer-1).GetName());
}

void LayoutEditorCanvas::SendSelectionToLayer(const std::string & newLayerName)
{
    for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
    {
        if (it->first == NULL) continue;

        it->first->SetLayer(newLayerName);
    }

    ChangesMade();
    for (std::set<gd::LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->InitialInstancesUpdated();
}

void LayoutEditorCanvas::OnPropObjSelected(wxCommandEvent & event)
{
    parentAuiManager->GetPane("PROPERTIES").Show();
    parentAuiManager->Update();
}

/** \brief Tool class picking the smallest instance under the cursor.
 */
class HighestZOrderFinder : public gd::InitialInstanceFunctor
{
public:
    HighestZOrderFinder() : highestZOrder(0), firstCall(true), layerRestricted(false) {};
    virtual ~HighestZOrderFinder() {};

    virtual void operator()(gd::InitialInstance & instance)
    {
        if ( !layerRestricted || instance.GetLayer() == layerName)
        {
            if ( firstCall ) highestZOrder = instance.GetZOrder();
            else highestZOrder = std::max(highestZOrder, instance.GetZOrder());
        }
    }

    void RestrictSearchToLayer(const std::string & layerName_) { layerName = layerName_; layerRestricted = true; };
    int GetHighestZOrder() const { return highestZOrder; }

private:
    int highestZOrder;
    bool firstCall;

    bool layerRestricted; ///< If true, the search is restricted to the layer called \a layerName.
    std::string layerName;
};

void LayoutEditorCanvas::AddObject(const std::string & objectName)
{
    AddObject(objectName, GetMouseXOnLayout(), GetMouseYOnLayout());
}

void LayoutEditorCanvas::AddObject(const std::string & objectName, float x, float y)
{
    if ( !editing || objectName.empty() ) return;
    isMovingInstance = false;

    //Create the new instance
    InitialInstance & newInstance = instances.InsertNewInitialInstance();
    newInstance.SetObjectName(objectName);
    newInstance.SetLayer(currentLayer);

    //Compute position
    if ( options.grid && options.snap )
    {
        newInstance.SetX(static_cast<int>(x/options.gridWidth +0.5)*options.gridWidth);
        newInstance.SetY(static_cast<int>(y/options.gridHeight+0.5)*options.gridHeight);
    }
    else
    {
        newInstance.SetX(x);
        newInstance.SetY(y);
    }

    //Compute the Z order
    HighestZOrderFinder zOrderFinder;
    zOrderFinder.RestrictSearchToLayer(currentLayer);
    instances.IterateOverInstances(zOrderFinder);
    newInstance.SetZOrder(zOrderFinder.GetHighestZOrder()+1);

    //Notify sub editors
    for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->InitialInstancesUpdated();
    ChangesMade();
}

void LayoutEditorCanvas::OnLeftDown( wxMouseEvent &event )
{
    SetFocus();

    if ( !editing ) return;

    if ( hasJustRightClicked )
    {
        hasJustRightClicked = false;
        return;
    }

    double mouseX = GetMouseXOnLayout();
    double mouseY = GetMouseYOnLayout();

    //Check if there is a click on a gui element inside the layout
    for (unsigned int i = 0;i<guiElements.size();++i)
    {
        if ( guiElements[i].area.Contains(event.GetX(), event.GetY()) )
        {
            OnGuiElementPressed(guiElements[i]);
            return ;
        }
    }

    //Check if an instance is selected
    {
        InitialInstance * instance = GetInitialInstanceUnderCursor();

        //Check if we must unselect all the objects
        if ( !shiftPressed && //Check that shift is not pressed
            ( instance == NULL || //If no object is clicked
              selectedInstances.find(instance) == selectedInstances.end()) ) //Or an object which is not currently selected.
        {
            ClearSelection();
        }


        if ( instance == NULL ) //If no object is clicked, create a selection rectangle
        {
            isSelecting = true;
            selectionRectangle = wxRect(wxPoint(mouseX, mouseY), wxPoint(mouseX, mouseY));
            return;
        }
        else //We made a click on an object
        {
            SelectInstance(instance);

            if (!isMovingInstance && ctrlPressed) //Clone objects
            {
                std::vector < InitialInstance* > selection = GetSelection();
                for (unsigned int i = 0;i<selection.size();++i)
                    instances.InsertInitialInstance(*selection[i]);

                for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
                    (*it)->InitialInstancesUpdated();
            }

            isMovingInstance = true;
        }

        oldMouseX = mouseX; //Remember the old position of the cursor for
        oldMouseY = mouseY; //use during the next event.
    }
}

void LayoutEditorCanvas::OnRightUp( wxMouseEvent &event )
{
    #if defined(LINUX) //Simulate click on linux
    sf::Event myEvent;
    myEvent.type = sf::Event::MouseButtonReleased;
    myEvent.mouseButton.x = event.GetX();
    myEvent.mouseButton.y = event.GetY();
    myEvent.mouseButton.button = sf::Mouse::Right;

    layout.GetRenderTargetEvents().push_back(myEvent);
    #endif

    if ( !editing ) return;


    //Check if an instance is selected
    {
        gd::InitialInstance * instance = GetInitialInstanceUnderCursor();

        double mouseX = GetMouseXOnLayout();
        double mouseY = GetMouseYOnLayout();
        oldMouseX = mouseX; //Remember the old position of the cursor for
        oldMouseY = mouseY; //use during the next event.

        //Check if we must unselect all the objects
        if ( !shiftPressed && //Check that shift is not pressed
            ( instance == NULL || //If no object is clicked
              selectedInstances.find(instance) == selectedInstances.end()) ) //Or an object which is not currently selected.
        {
            ClearSelection();
        }

        //Display the appropriate context menu
        if ( instance != NULL )
        {
            SelectInstance(instance);
            OnUpdate(); //So as to display selection rectangle for the newly selected object
            UpdateContextMenu();
            PopupMenu(&contextMenu);
        }
        else
        {
            //Check if there is locked instance under the cursor.
            gd::InitialInstance * instance = GetInitialInstanceUnderCursor(/*pickOnlyLockedInstances=*/true);
            noObjectContextMenu.Enable(ID_UNLOCKMENU, instance != NULL);

            PopupMenu(&noObjectContextMenu);
        }

    }
}

void LayoutEditorCanvas::ClearSelection()
{
    selectedInstances.clear();
    for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->DeselectedAllInitialInstance();
}

void LayoutEditorCanvas::SelectInstance(InitialInstance * instance)
{
    if ( !instance ) return;

    selectedInstances[instance] = wxRealPoint(instance->GetX(), instance->GetY());
    for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->SelectedInitialInstance(*instance);
}

void LayoutEditorCanvas::UnselectInstance(InitialInstance * instance)
{
    if ( !instance ) return;

    selectedInstances.erase(instance);
    for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->DeselectedInitialInstance(*instance);
}

void LayoutEditorCanvas::DeleteInstances(std::vector<InitialInstance *> instancesToDelete)
{
    for (unsigned int i = 0;i<instancesToDelete.size();++i)
    {
        if (instancesToDelete[i] == NULL ) continue;

        instances.RemoveInstance(*instancesToDelete[i]);

        if ( selectedInstances.find(instancesToDelete[i]) != selectedInstances.end()) selectedInstances.erase(instancesToDelete[i]);
    }

    for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
        (*it)->InitialInstancesUpdated();
}

/** \brief Tool class collecting in a list all the instances that are inside the selectionRectangle of the layout editor canvas.
 */
class InstancesInsideSelectionPicker : public gd::InitialInstanceFunctor
{
public:
    InstancesInsideSelectionPicker(const LayoutEditorCanvas & editor_) : editor(editor_) {};
    virtual ~InstancesInsideSelectionPicker() {};

    virtual void operator()(gd::InitialInstance & instance)
    {
        if ( instance.IsLocked() ) return;

        sf::Vector2f size = editor.GetInitialInstanceSize(instance);
        sf::Vector2f origin = editor.GetInitialInstanceOrigin(instance);

        if ( editor.selectionRectangle.Contains(instance.GetX()-origin.x, instance.GetY()-origin.y) &&
             editor.selectionRectangle.Contains(instance.GetX()-origin.x+size.x,
                                                instance.GetY()-origin.y+size.y) )
        {
            selectedList.push_back(&instance);
        }
    }

    std::vector<InitialInstance*> & GetSelectedList() { return selectedList; };

private:
    const LayoutEditorCanvas & editor;
    std::vector<InitialInstance*> selectedList; ///< This list will be filled with the instances that are into the selectionRectangle
};

void LayoutEditorCanvas::OnLeftUp( wxMouseEvent &event )
{
    #if defined(LINUX) //Simulate click on linux
    sf::Event myEvent;
    myEvent.type = sf::Event::MouseButtonReleased;
    myEvent.mouseButton.x = event.GetX();
    myEvent.mouseButton.y = event.GetY();
    myEvent.mouseButton.button = sf::Mouse::Left;

    layout.GetRenderTargetEvents().push_back(myEvent);
    #endif

    if ( !editing ) return;

    if ( !currentDraggableBt.empty() ) //First check if we were dragging a button.
    {
        currentDraggableBt.clear();

        if ( currentDraggableBt.substr(0, 6) == "resize" ) //Handle the release of resize buttons here ( as the mouse if not necessarily on the button so OnGuiButtonReleased is not called )
        {
            for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
            {
                it->second.x = it->first->GetX(); it->second.y = it->first->GetY();
            }
        }
        return;
    }

    //Check if there is a click released on a gui element inside the layout
    for (unsigned int i = 0;i<guiElements.size();++i)
    {
        if ( guiElements[i].area.Contains(event.GetX(), event.GetY()) )
        {
            OnGuiElementReleased(guiElements[i]);
            return;
        }
    }

    if ( isMovingInstance )
    {
        bool changesMade = false;
        for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        {
            //Update the member containing the "start" position of the instances.
            if (it->second.x != it->first->GetX() || it->second.y != it->first->GetY() )
            {
                it->second.x = it->first->GetX(); it->second.y = it->first->GetY();
                changesMade = true;
            }
        }

        if ( changesMade )
        {
            ChangesMade();

            for (std::set<LayoutEditorCanvasAssociatedEditor*>::iterator it = associatedEditors.begin();it !=associatedEditors.end();++it)
                (*it)->InitialInstancesUpdated();
        }
        isMovingInstance = false;
    }

    //Select object thanks to the selection area
    if ( isSelecting )
    {
        //Be sure that the selection rectangle origin is on the top left
        if ( selectionRectangle.GetWidth() < 0 )
        {
            selectionRectangle.SetX(selectionRectangle.GetX()+selectionRectangle.GetWidth());
            selectionRectangle.SetWidth(-selectionRectangle.GetWidth());
        }
        if ( selectionRectangle.GetHeight() < 0 )
        {
            selectionRectangle.SetY(selectionRectangle.GetY()+selectionRectangle.GetHeight());
            selectionRectangle.SetHeight(-selectionRectangle.GetHeight());
        }

        //Select the instances that are inside the selection rectangle
        InstancesInsideSelectionPicker picker(*this);
        instances.IterateOverInstances(picker);

        for ( unsigned int i = 0; i<picker.GetSelectedList().size();++i)
            SelectInstance(picker.GetSelectedList()[i]);

        isSelecting = false;
    }
}

void LayoutEditorCanvas::OnMotion( wxMouseEvent &event )
{
    //First check if we're using a resize button
    if ( currentDraggableBt.substr(0,6) == "resize")
    {
        if ( currentDraggableBt == "resizeRight" || currentDraggableBt == "resizeRightUp" || currentDraggableBt == "resizeRightDown" )
        {
            for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
            {
                if (resizeOriginalWidths[it->first]+GetMouseXOnLayout()-resizeMouseStartPosition.x < 0) continue;

                if ( !it->first->HasCustomSize() ) {
                    it->first->SetHasCustomSize(true);
                    it->first->SetCustomHeight(resizeOriginalHeights[it->first]);
                }
                it->first->SetCustomWidth(resizeOriginalWidths[it->first]+GetMouseXOnLayout()-resizeMouseStartPosition.x);
            }
        }
        if ( currentDraggableBt == "resizeDown" || currentDraggableBt == "resizeRightDown" || currentDraggableBt == "resizeLeftDown" )
        {
            for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
            {
                if ( resizeOriginalHeights[it->first]+GetMouseYOnLayout()-resizeMouseStartPosition.y < 0 ) continue;

                if ( !it->first->HasCustomSize() ) {
                    it->first->SetHasCustomSize(true);
                    it->first->SetCustomWidth(resizeOriginalWidths[it->first]);
                }
                it->first->SetCustomHeight(resizeOriginalHeights[it->first]+GetMouseYOnLayout()-resizeMouseStartPosition.y);
            }
        }
        if ( currentDraggableBt == "resizeLeft" || currentDraggableBt == "resizeLeftUp" || currentDraggableBt == "resizeLeftDown" )
        {
            for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
            {
                if (resizeOriginalWidths[it->first]-GetMouseXOnLayout()+resizeMouseStartPosition.x < 0) continue;

                if ( !it->first->HasCustomSize() ) {
                    it->first->SetHasCustomSize(true);
                    it->first->SetCustomHeight(resizeOriginalHeights[it->first]);
                }
                it->first->SetCustomWidth(resizeOriginalWidths[it->first]-GetMouseXOnLayout()+resizeMouseStartPosition.x);
                it->first->SetX(it->second.x+GetMouseXOnLayout()-resizeMouseStartPosition.x);
            }
        }
        if ( currentDraggableBt == "resizeUp" || currentDraggableBt == "resizeLeftUp" || currentDraggableBt == "resizeRightUp" )
        {
            for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
            {
                if ( resizeOriginalHeights[it->first]-GetMouseYOnLayout()+resizeMouseStartPosition.y < 0 ) continue;

                if ( !it->first->HasCustomSize() ) {
                    it->first->SetHasCustomSize(true);
                    it->first->SetCustomWidth(resizeOriginalWidths[it->first]);
                }
                it->first->SetCustomHeight(resizeOriginalHeights[it->first]-GetMouseYOnLayout()+resizeMouseStartPosition.y);
                it->first->SetY(it->second.y+GetMouseYOnLayout()-resizeMouseStartPosition.y);
            }
        }

        UpdateMouseResizeCursor(currentDraggableBt);
    }
    else if (currentDraggableBt == "angle") //Check if we are dragging a angle button
    {
        for ( std::map <gd::InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        {
            float newAngle = atan2(sf::Mouse::getPosition(*this).y-angleButtonCenter.y, sf::Mouse::getPosition(*this).x-angleButtonCenter.x)*180/3.14159;
            it->first->SetAngle(newAngle);
        }
    }
    else //No buttons being used
    {
        //Moving using middle click
        if ( isMovingView )
        {
            float zoomFactor = static_cast<float>(getSize().x)/editionView.getSize().x;

            editionView.setCenter( movingViewStartPosition + (movingViewMouseStartPosition - sf::Vector2f(sf::Mouse::getPosition(*this)))/zoomFactor );
        }

        double mouseX = GetMouseXOnLayout();
        double mouseY = GetMouseYOnLayout();

        if ( !editing )
            wxLogStatus( wxString( _( "Position " ) ) + ToString( mouseX ) + wxString( _( ";" ) ) + ToString( mouseY ) + wxString( _( ". ( Base layer, camera 0 )" ) ) );
        else
            wxLogStatus( wxString( _( "Position " ) ) + ToString( mouseX ) + wxString( _( ";" ) ) + ToString( mouseY ) + wxString( _( ". SHIFT for multiple selection, right click for more options." ) ) );

        //Check if there is a gui element hovered inside the layout
        for (unsigned int i = 0;i<guiElements.size();++i)
        {
            if ( guiElements[i].area.Contains(event.GetX(), event.GetY()) )
                OnGuiElementHovered(guiElements[i]);
        }

        if ( isMovingInstance )
        {
            //Get the displacement of the cursor
            float deltaX = mouseX - oldMouseX;
            float deltaY = mouseY - oldMouseY;

            for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
            {
                //Compute new position
                float newX = it->second.x + deltaX;
                float newY = it->second.y + deltaY;

                if ( options.grid && options.snap )
                {
                    newX = std::floor(newX/options.gridWidth +0.5)*options.gridWidth;
                    newY = std::floor(newY/options.gridHeight+0.5)*options.gridHeight;
                }

                //Move the initial instance
                it->first->SetX(newX);
                it->first->SetY(newY);
            }
        }
        if ( isSelecting )
        {
            selectionRectangle.SetBottomRight(wxPoint(mouseX, mouseY));
        }
    }
}

void LayoutEditorCanvas::OnMiddleDown( wxMouseEvent &event )
{
    if ( !editing ) return;

    //User can move the view thanks to middle click
    if ( !isMovingView )
    {
        isMovingView = true;
        movingViewMouseStartPosition = sf::Vector2f(sf::Mouse::getPosition(*this));
        movingViewStartPosition = getView().getCenter();
        SetCursor( wxCursor( wxCURSOR_SIZING ) );

        return;
    }
    else
    {
        isMovingView = false;
        SetCursor( wxNullCursor );
    }
}


void LayoutEditorCanvas::OnLeftDClick( wxMouseEvent &event )
{
    if ( !editing ) return;

    parentAuiManager->GetPane("PROPERTIES").Show();
    parentAuiManager->Update();
}

void LayoutEditorCanvas::OnKey( wxKeyEvent& evt )
{
    if (!editing)
    {
        evt.StopPropagation();
        return;
    }

    if ( evt.GetKeyCode() == WXK_CONTROL )
        ctrlPressed = true;
    if ( evt.GetKeyCode() == WXK_SHIFT )
        shiftPressed = true;
    if ( evt.GetKeyCode() == WXK_ALT )
        altPressed = true;

    if ( evt.GetKeyCode() == WXK_DELETE )
    {
        std::vector<InitialInstance*> instancesToDelete;
        for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
            instancesToDelete.push_back(it->first);

        DeleteInstances(instancesToDelete);

        ClearSelection();
        ChangesMade();
    }
    else if ( evt.GetKeyCode() == WXK_DOWN )
    {
        for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        {
            it->first->SetY(it->first->GetY()+1);
        }
    }
    else if ( evt.GetKeyCode() == WXK_UP )
    {
        for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        {
            it->first->SetY(it->first->GetY()-1);
        }
    }
    else if ( evt.GetKeyCode() == WXK_RIGHT )
    {
        for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        {
            it->first->SetX(it->first->GetX()+1);
        }
    }
    else if ( evt.GetKeyCode() == WXK_LEFT )
    {
        for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        {
            it->first->SetX(it->first->GetX()-1);
        }
    }

    evt.StopPropagation();
}

void LayoutEditorCanvas::OnKeyUp( wxKeyEvent& evt )
{
    if (!editing)
    {
        evt.StopPropagation();
        return;
    }

    if ( evt.GetKeyCode() == WXK_CONTROL )
        ctrlPressed = false;
    if ( evt.GetKeyCode() == WXK_SHIFT )
        shiftPressed = false;
    if ( evt.GetKeyCode() == WXK_ALT )
        altPressed = false;
}

void LayoutEditorCanvas::ChangesMade()
{
    history.push_back(boost::shared_ptr<gd::InitialInstancesContainer>(latestState->Clone()));
    redoHistory.clear();
    latestState->Create(instances);
}


/** \brief Tool class picking the smallest instance under the cursor.
 */
class SmallestInstanceUnderCursorPicker : public gd::InitialInstanceFunctor
{
public:
    SmallestInstanceUnderCursorPicker(const LayoutEditorCanvas & editor_, double xPosition_, double yPosition_) :
        editor(editor_),
        smallestInstance(NULL),
        smallestInstanceArea(0),
        xPosition(xPosition_),
        yPosition(yPosition_),
        pickLockedOnly(false)
    {
    };
    virtual ~SmallestInstanceUnderCursorPicker() {};

    virtual void operator()(gd::InitialInstance & instance)
    {
        if ( pickLockedOnly != instance.IsLocked() ) return;

        sf::Vector2f size = editor.GetInitialInstanceSize(instance);
        sf::Vector2f origin = editor.GetInitialInstanceOrigin(instance);

        wxRect2DDouble boundingBox(instance.GetX()-origin.x, instance.GetY()-origin.y, size.x, size.y);

        if ( boundingBox.Contains(wxPoint2DDouble(xPosition, yPosition)) )
        {
            if ( smallestInstance == NULL || boundingBox.GetSize().GetWidth()*boundingBox.GetSize().GetHeight() < smallestInstanceArea )
            {
                smallestInstance = &instance;
                smallestInstanceArea = boundingBox.GetSize().GetWidth()*boundingBox.GetSize().GetHeight();
            }
        }
    }

    InitialInstance * GetSmallestInstanceUnderCursor() { return smallestInstance; };

    void PickLockedInstancesAndOnlyThem() { pickLockedOnly = true; }

private:
    const LayoutEditorCanvas & editor;
    InitialInstance * smallestInstance;
    double smallestInstanceArea;
    const double xPosition;
    const double yPosition;
    bool pickLockedOnly;
};

InitialInstance * LayoutEditorCanvas::GetInitialInstanceAtPosition(double xPosition, double yPosition, bool pickOnlyLockedInstances)
{
    SmallestInstanceUnderCursorPicker picker(*this, xPosition, yPosition);
    if ( pickOnlyLockedInstances ) picker.PickLockedInstancesAndOnlyThem();
    instances.IterateOverInstances(picker);

    return picker.GetSmallestInstanceUnderCursor();
}

std::vector<gd::InitialInstance*> LayoutEditorCanvas::GetSelection()
{
    std::vector<gd::InitialInstance*> selection;
    for ( std::map <InitialInstance*, wxRealPoint >::iterator it = selectedInstances.begin();it!=selectedInstances.end();++it)
        selection.push_back(it->first);

    return selection;
}

void LayoutEditorCanvas::OnLayersEditor( wxCommandEvent & event )
{
    parentAuiManager->GetPane("EL").Show();
    parentAuiManager->Update();
}

void LayoutEditorCanvas::OnObjectsEditor( wxCommandEvent & event )
{
    parentAuiManager->GetPane("EO").Show();
    parentAuiManager->Update();
}

void LayoutEditorCanvas::OnObjectsPositionList( wxCommandEvent & event )
{
    parentAuiManager->GetPane("InstancesBrowser").Show();
    parentAuiManager->Update();
}

void LayoutEditorCanvas::OnGridBtClick( wxCommandEvent & event )
{
    options.grid = !options.grid;
}

void LayoutEditorCanvas::OnGridSetupBtClick( wxCommandEvent & event )
{
    GridSetupDialog dialog(this, options.gridWidth, options.gridHeight, options.snap, options.gridR, options.gridG, options.gridB);
    dialog.ShowModal();
}

void LayoutEditorCanvas::OnWindowMaskBtClick( wxCommandEvent & event )
{
    options.windowMask = !options.windowMask;
}

void LayoutEditorCanvas::OnUndoMoreBtClick(wxRibbonButtonBarEvent& evt)
{
    evt.PopupMenu(&undoMenu);
}

void LayoutEditorCanvas::OnUndoBtClick( wxCommandEvent & event )
{
    Undo();
}

void LayoutEditorCanvas::Undo(unsigned int times)
{
    for (unsigned int i = 0;i<times;++i)
    {
        if ( history.empty() ) return;

        redoHistory.push_back(boost::shared_ptr<gd::InitialInstancesContainer>(instances.Clone()));
        instances.Create(*history.back());
        history.pop_back();

        latestState = boost::shared_ptr<gd::InitialInstancesContainer>(instances.Clone());
    }
}

void LayoutEditorCanvas::OnClearHistorySelected(wxCommandEvent& event)
{
    if (wxMessageBox("Etes-vous s�r de vouloir supprimer l'historique des modifications ?", "�tes vous s�r ?",wxYES_NO ) != wxYES)
        return;

    history.clear();
    redoHistory.clear();
}

void LayoutEditorCanvas::Redo( unsigned int times )
{
    for (unsigned int i = 0;i<times;++i)
    {
        if ( redoHistory.empty() ) return;

        history.push_back(boost::shared_ptr<gd::InitialInstancesContainer>(instances.Clone()));
        instances.Create(*redoHistory.back());
        redoHistory.pop_back();

        latestState = boost::shared_ptr<gd::InitialInstancesContainer>(instances.Clone());
    }
}

void LayoutEditorCanvas::OnRedoBtClick( wxCommandEvent & event )
{
    Redo();
}

/**
 * Toggle Game Develop full screen mode.
 */
void LayoutEditorCanvas::OnFullScreenBtClick(wxCommandEvent & event)
{
    if (!mainFrameWrapper.GetMainEditor()->IsFullScreen())
        mainFrameWrapper.GetMainEditor()->ShowFullScreen(true, wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
    else
        mainFrameWrapper.GetMainEditor()->ShowFullScreen(false);
}

double LayoutEditorCanvas::GetMouseXOnLayout() const
{
    return convertCoords(sf::Mouse::getPosition(*this), editionView).x;
}

double LayoutEditorCanvas::GetMouseYOnLayout() const
{
    return convertCoords(sf::Mouse::getPosition(*this), editionView).y;
}

sf::Vector2f LayoutEditorCanvas::GetInitialInstanceSize(gd::InitialInstance & instance) const
{
    if (instance.HasCustomSize()) return sf::Vector2f(instance.GetCustomWidth(), instance.GetCustomHeight());

    gd::Object * object = GetObjectLinkedToInitialInstance(instance);
    if ( object ) return object->GetInitialInstanceDefaultSize(instance, project, layout);

    return sf::Vector2f(32,32);
}

sf::Vector2f LayoutEditorCanvas::GetInitialInstanceOrigin(gd::InitialInstance & instance) const
{
    gd::Object * object = GetObjectLinkedToInitialInstance(instance);
    if ( object ) return object->GetInitialInstanceOrigin(instance, project, layout);

    return sf::Vector2f(0,0);
}

gd::Object * LayoutEditorCanvas::GetObjectLinkedToInitialInstance(gd::InitialInstance & instance) const
{
    if ( layout.HasObjectNamed(instance.GetObjectName()) )
        return &layout.GetObject(instance.GetObjectName());
    else if ( project.HasObjectNamed(instance.GetObjectName()) )
        return &project.GetObject(instance.GetObjectName());

    return NULL;
}

void LayoutEditorCanvas::UpdateMouseResizeCursor(const std::string & currentDraggableBt)
{
    if ( currentDraggableBt == "resizeUp" || currentDraggableBt == "resizeDown"  )
        wxSetCursor(wxCursor(wxCURSOR_SIZENS));
    if ( currentDraggableBt == "resizeLeft" || currentDraggableBt == "resizeRight"  )
        wxSetCursor(wxCursor(wxCURSOR_SIZEWE));
    if ( currentDraggableBt == "resizeLeftUp" || currentDraggableBt == "resizeRightDown"  )
        wxSetCursor(wxCursor(wxCURSOR_SIZENWSE));
    if ( currentDraggableBt == "resizeRightUp" || currentDraggableBt == "resizeLeftDown"  )
        wxSetCursor(wxCursor(wxCURSOR_SIZENESW));
}

bool LayoutEditorCanvas::PreviewPaused() const
{
    return !editing && (currentPreviewer ? currentPreviewer->IsPaused() : false);
}

void LayoutEditorCanvas::PausePreview()
{
    if ( editing ) return;

    if (currentPreviewer) currentPreviewer->PausePreview();
}

void LayoutEditorCanvas::SetParentAuiManager(wxAuiManager * parentAuiManager_)
{
    parentAuiManager = parentAuiManager_;
    for(std::map<std::string, boost::shared_ptr<gd::LayoutEditorPreviewer> >::iterator it = previewers.begin();
        it != previewers.end();
        ++it)
    {
        if ( it->second != boost::shared_ptr<gd::LayoutEditorPreviewer>() ) { it->second->SetParentAuiManager(parentAuiManager_); }
    }
}

void LayoutEditorCanvas::ReloadResources()
{
    wxString oldWorkingDir = wxGetCwd();

    if ( wxDirExists(wxFileName::FileName(project.GetProjectFile()).GetPath()))
        wxSetWorkingDirectory(wxFileName::FileName(project.GetProjectFile()).GetPath());

    for (unsigned int i = 0;i<layout.GetObjectsCount();++i)
        layout.GetObject(i).LoadResources(project, layout);
    for (unsigned int i = 0;i<project.GetObjectsCount();++i)
        project.GetObject(i).LoadResources(project, layout);

    if ( wxDirExists(oldWorkingDir))
        wxSetWorkingDirectory(oldWorkingDir);
}

void LayoutEditorCanvas::GoToEditingState()
{
    wxCommandEvent useless;
    OnEditionBtClick(useless);
}

}

//The rest of the implementation is available in LayoutEditorCanvas2.cpp