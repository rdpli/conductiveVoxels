#ifndef VOXCAD_H
#define VOXCAD_H

#include <QtGui/QMainWindow>
#include "ui_VoxCad.h"
#include "QVX_Edit.h"
#include "QVX_Interfaces.h"
#include "QOpenGL.h"

#include <QDockWidget>
#include "Dlg_Palette.h"
#include "Dlg_Workspace.h"
#include "Dlg_VoxInfo.h"
#include "Dlg_BCs.h"
#include "Dlg_FEAInfo.h"
#include "Dlg_Physics.h"

#ifdef DMU_ENABLED
#include "DMU.h"
#endif


class CQOpenGL;

//#define TINY_XML //use tiny xml library?
#define QT_XML //use qt XML library?

class VoxCad : public QMainWindow
{
	Q_OBJECT

public:
	VoxCad(QWidget *parent = 0, Qt::WFlags flags = 0);
	~VoxCad();

	enum AppViewMode {VM_3DVIEW, VM_EDITLAYER, VM_EDITBCS, VM_FEA, VM_PHYSICS, VM_NONE};
	AppViewMode CurViewMode;
	void EnterVMMode(AppViewMode NewMode, bool Force = false); //switch between viewing modes!

	int CurGLSelected; //index of currently selected voxel

	CQOpenGL* GLWindow;
	CQDM_Edit MainObj;
	QVX_FEA MainFEA;
	QVX_Environment MainEnv;
	QVX_Sim MainSim;
	
	//stinks to have to do this here, but no way to do it in QT designer it seems...
	QActionGroup* DrawGroup;

	void SetupGLWindow(void);

	//create dock widgets programatically
	//Palette Dialog:
	void SetupPaletteWindow(void);
    QDockWidget *PaletteDockWidget;
	Dlg_Palette *PaletteDlg;

	//Workspace Dialog:
	void SetupWorkspaceWindow(void);
    QDockWidget *WorkspaceDockWidget;
	Dlg_Workspace *WorkspaceDlg;

	//3D Reference Dialog
	void SetupRef3DWindow(void);
	QDockWidget *Ref3DDockWidget;
	CQOpenGL* GLRef3DWin;

	//Voxel Info Dialog
	void SetupVoxInfoWindow(void);
	QDockWidget *VoxInfoDockWidget;
	Dlg_VoxInfo *VoxInfoDlg;

	//Boundary Conditions Dialog
	void SetupBCWindow(void);
	QDockWidget *BCDockWidget;
	Dlg_BCs *BCsDlg;

	//FEA Info Dialog
	void SetupFEAInfoWindow(void);
	QDockWidget *FEAInfoDockWidget;
	Dlg_FEAInfo *FEAInfoDlg;

	// Physics Sandbox Dialog
	void SetupPhysicsWindow(void);
	QDockWidget *PhysicsDockWidget;
	Dlg_Physics *PhysicsDlg;


public slots: //global slot repository for updating things across all windows/views...
	//File
	void New(){ForceViewMode(); MainObj.New(); UpdateAllWins();};
	void OpenVXC(){ForceViewMode(); MainObj.Open(); UpdateAllWins();}
	void ImportVXA(){ForceViewMode(); MainSim.OpenVXA(); UpdateAllWins();}

	//Edit
	void Copy(void);
	void Cut(void);
	void Paste(void);

	//View
	void ViewPerspective(bool RedrawNow = true){GLWindow->SetViewCustom1(); if (RedrawNow) ReqGLUpdateAll();};
	void ViewTop(bool RedrawNow = true){MainObj.SetV2DTop(); GLWindow->SetViewTop(); if (RedrawNow) ReqGLUpdateAll();};
	void ViewBottom(void){MainObj.SetV2DBottom(); GLWindow->SetViewBottom(); ReqGLUpdateAll();};
	void ViewLeft(void){MainObj.SetV2DLeft(); GLWindow->SetViewLeft(); ReqGLUpdateAll();};
	void ViewRight(void){MainObj.SetV2DRight(); GLWindow->SetViewRight(); ReqGLUpdateAll();};
	void ViewFront(void){MainObj.SetV2DFront(); GLWindow->SetViewFront(); ReqGLUpdateAll();};
	void ViewBack(void){MainObj.SetV2DBack(); GLWindow->SetViewBack(); ReqGLUpdateAll();};

	void UpdateAllWins() {EnterVMMode(VM_3DVIEW); PaletteDlg->UpdateUI(); WorkspaceDlg->IniUpdateUI(); VoxInfoDlg->UpdateUI(); ReqGLUpdateAll();}

	void ReqGLUpdateAll() {if (GLWindow->isVisible()) GLWindow->updateGL(); if (GLRef3DWin->isVisible()) GLRef3DWin->updateGL();}
	void ZoomExtAll() {if (GLWindow->isVisible()) GLWindow->ZoomExtents(); if (GLRef3DWin->isVisible()) GLRef3DWin->ZoomExtents();}

	void SetGLSelected(int NewSel=-1) {if (NewSel != CurGLSelected) {CurGLSelected=NewSel; ReqGLUpdateAll(); BCsDlg->ApplyExtSelection(NewSel); VoxInfoDlg->UpdateUI(); FEAInfoDlg->UpdateUI(); PhysicsDlg->ApplyVoxSelection(NewSel);}};
	void GetCurGLSelected(int* CurSel) {*CurSel = CurGLSelected;}

	void WSDimChanged() {ZoomExtAll(); SetGLSelected();};
	void SetSectionView(bool ViewSec);
	
	void DrawCurScene(void);
	//change modes:
	void ViewMode(void) {EnterVMMode(VM_3DVIEW);};
	void ForceViewMode(void) {EnterVMMode(VM_3DVIEW, true);} //forces the mode to view mode without questions...
	void EditMode(bool entering=true) {if (entering) EnterVMMode(VM_EDITLAYER); else EnterVMMode(VM_3DVIEW);};
	void BCsMode(bool entering=true) {if(entering) EnterVMMode(VM_EDITBCS); else EnterVMMode(VM_3DVIEW);};
	void FEAMode(bool entering=true) {if(entering) EnterVMMode(VM_FEA); else {EnterVMMode(VM_3DVIEW); ui.actionSolve->setChecked(false);}};
	void RequestFEAMode(bool entering = true){if(entering) MainFEA.RequestSolveSystem(); else EnterVMMode(VM_3DVIEW);} //calls back with SolveSuccess signal
	void PhysicsMode(bool entering=true) {if(entering) EnterVMMode(VM_PHYSICS); else EnterVMMode(VM_3DVIEW);};

	//info for OpenGL class
	void WantGLIndex(bool* YN) {if(CurViewMode!=VM_EDITLAYER) *YN=true; else *YN = false;}
	void WantCoord3D(bool* YN) {if((CurViewMode==VM_EDITLAYER && GLWindow->GetCurView() != VPERSPECTIVE )|| CurViewMode==VM_PHYSICS) *YN=true; else *YN = false;}

	//mouse handlers: (take the emits from OpenGL and distribute them according to current mode:
	void HoverMove(float X, float Y, float Z) {	if(CurViewMode==VM_EDITLAYER) MainObj.HoverMove(Vec3D(X, Y, Z));};
	void LMouseDown(float X, float Y, float Z) {if(CurViewMode==VM_EDITLAYER) MainObj.LMouseDown(Vec3D(X, Y, Z)); else if (CurViewMode==VM_PHYSICS) MainSim.LMouseDown(Vec3D(X, Y, Z));};
	void LMouseUp(float X, float Y, float Z) {if(CurViewMode==VM_EDITLAYER) MainObj.LMouseUp(Vec3D(X, Y, Z));else if (CurViewMode==VM_PHYSICS) MainSim.LMouseUp(Vec3D(X, Y, Z));};
	void LMouseDownMove(float X, float Y, float Z) {if(CurViewMode==VM_EDITLAYER) MainObj.LMouseDownMove(Vec3D(X, Y, Z)); else if (CurViewMode==VM_PHYSICS) MainSim.LMouseDownMove(Vec3D(X, Y, Z));};
	void PressedEscape(void) {if(CurViewMode==VM_EDITLAYER) MainObj.PressedEscape(); else ZoomExtAll();};
	void CtrlMouseRoll(bool Positive) {if(CurViewMode==VM_EDITLAYER) MainObj.CtrlMouseRoll(Positive);};

	//Hide and show dialog/windows (and keep checked stuff in sync!
	void ViewPaletteWindow(bool Visible) {if (Visible){PaletteDockWidget->show();PaletteDockWidget->raise(); ui.actionPalette->setChecked(true);} else {PaletteDockWidget->hide();ui.actionPalette->setChecked(false);}};
	void ViewWorkspaceWindow(bool Visible){if (Visible){WorkspaceDockWidget->show();WorkspaceDockWidget->raise();ui.actionWorkspace->setChecked(true);} else {WorkspaceDockWidget->hide();ui.actionWorkspace->setChecked(false);}};
	void ViewRef3DWindow(bool Visible) {if (Visible){Ref3DDockWidget->show();ui.actionReference_View->setChecked(true);} else {Ref3DDockWidget->hide();ui.actionReference_View->setChecked(false);}};
	void ViewVoxInfoWindow(bool Visible){if (Visible){VoxInfoDockWidget->show();VoxInfoDockWidget->raise();ui.actionInfo->setChecked(true);} else {VoxInfoDockWidget->hide();ui.actionInfo->setChecked(false);}};
	void ViewBCsWindow(bool Visible){if (Visible){BCDockWidget->show();BCDockWidget->raise();ui.actionBCs->setChecked(true);} else {BCDockWidget->hide();ui.actionBCs->setChecked(false);}};
	void ViewFEAInfoWindow(bool Visible){if (Visible){FEAInfoDockWidget->show();FEAInfoDockWidget->raise();ui.actionSolve->setChecked(true);} else {FEAInfoDockWidget->hide();ui.actionSolve->setChecked(false);}};
	void ViewPhysicsWindow(bool Visible){if (Visible){PhysicsDockWidget->show();PhysicsDockWidget->raise();ui.actionPhysics->setChecked(true);} else {PhysicsDockWidget->hide();ui.actionPhysics->setChecked(false);}};

#ifdef DMU_ENABLED
	void ImportDMU(void) {OpenDMU(&MainObj); UpdateAllWins();};
	void ExportDMU(void) {SaveDMU(&MainObj);};
#endif

private:
	Ui::VoxCadClass ui;
};

#endif // VOXJET_TOUCH_H