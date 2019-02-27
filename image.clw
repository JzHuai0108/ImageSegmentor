; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CImageDoc
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "image.h"
LastPage=0

ClassCount=22
Class1=CBandSelDlg
Class2=CCannyPara
Class3=CChildFrame
Class4=CImageApp
Class5=CAboutDlg
Class6=CImageDoc
Class7=CImageView
Class8=CInitiateDlg
Class9=CMainFrame
Class10=CMFeatDlg
Class11=CMorphDlg
Class12=CMRFOptimDlg
Class13=CMyImageDBApp
Class14=CPreviewDlg
Class15=CResultDlg
Class16=CResultImageDlg
Class17=CSegmentPara
Class18=CSegParaPopDlg
Class19=CSelPropDlg
Class20=CSetPropDlg
Class21=CSUSANDlg
Class22=CTHRESHDlg

ResourceCount=18
Resource1=IDD_DIALOG_BANDSEL
Resource2=IDD_DIALOG_CANNY
Resource3=IDD_DIALOG_SUSAN
Resource4=IDD_DIALOG_MORPH
Resource5=IDD_DIALOG_PREVIEW
Resource6=IDD_DIALOG_SELPROP
Resource7=IDD_DIALOG_FEATUREEXTR
Resource8=IDD_DIALOG_CDTHRESH
Resource9=IDD_DIALOG_SETTHRESH
Resource10=IDD_DIALOG_RESULTIMAGE
Resource11=IDD_DIALOG_MRFOPTIM
Resource12=IDR_MAINFRAME (English (U.S.))
Resource13=IDR_MAINFRAME
Resource14=IDD_ABOUTBOX
Resource15=IDD_DIALOG_INITIATE
Resource16=IDD_DIALOG_SEGOUT
Resource17=IDD_DIALOG_IMAGEPROP
Resource18=IDR_IMAGETYPE (English (U.S.))

[CLS:CBandSelDlg]
Type=0
BaseClass=CDialog
HeaderFile=BandSelDlg.h
ImplementationFile=BandSelDlg.cpp

[CLS:CCannyPara]
Type=0
BaseClass=CDialog
HeaderFile=CannyPara.h
ImplementationFile=CannyPara.cpp

[CLS:CChildFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp

[CLS:CImageApp]
Type=0
BaseClass=CWinApp
HeaderFile=image.h
ImplementationFile=image.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=image.cpp
ImplementationFile=image.cpp
LastObject=ID_CLASS_MULTIFEATURE

[CLS:CImageDoc]
Type=0
BaseClass=CDocument
HeaderFile=imageDoc.h
ImplementationFile=imageDoc.cpp
LastObject=ID_CLASS_STICKCHAIN
Filter=N
VirtualFilter=DC

[CLS:CImageView]
Type=0
BaseClass=CScrollView
HeaderFile=imageView.h
ImplementationFile=imageView.cpp

[CLS:CInitiateDlg]
Type=0
BaseClass=CDialog
HeaderFile=InitiateDlg.h
ImplementationFile=InitiateDlg.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CMDIFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp

[CLS:CMFeatDlg]
Type=0
BaseClass=CDialog
HeaderFile=MFeatDlg.h
ImplementationFile=MFeatDlg.cpp

[CLS:CMorphDlg]
Type=0
BaseClass=CDialog
HeaderFile=MorphDlg.h
ImplementationFile=MorphDlg.cpp

[CLS:CMRFOptimDlg]
Type=0
BaseClass=CDialog
HeaderFile=MRFOptimDlg.h
ImplementationFile=MRFOptimDlg.cpp

[CLS:CMyImageDBApp]
Type=0
BaseClass=CWinApp
HeaderFile=MyImageDB.h
ImplementationFile=MyImageDB.cpp

[CLS:CPreviewDlg]
Type=0
BaseClass=CDialog
HeaderFile=PreviewDlg.h
ImplementationFile=PreviewDlg.cpp

[CLS:CResultDlg]
Type=0
BaseClass=CDialog
HeaderFile=ResultDlg.h
ImplementationFile=ResultDlg.cpp

[CLS:CResultImageDlg]
Type=0
BaseClass=CDialog
HeaderFile=resultimagedlg.h
ImplementationFile=resultimagedlg.cpp

[CLS:CSegmentPara]
Type=0
BaseClass=CDialog
HeaderFile=SegmentPara.h
ImplementationFile=SegmentPara.cpp

[CLS:CSegParaPopDlg]
Type=0
BaseClass=CDialog
HeaderFile=SegParaPopDlg.h
ImplementationFile=SegParaPopDlg.cpp

[CLS:CSelPropDlg]
Type=0
BaseClass=CDialog
HeaderFile=SelPropDlg.h
ImplementationFile=SelPropDlg.cpp

[CLS:CSetPropDlg]
Type=0
BaseClass=CDialog
HeaderFile=SetPropDlg.h
ImplementationFile=SetPropDlg.cpp

[CLS:CSUSANDlg]
Type=0
BaseClass=CDialog
HeaderFile=SUSANDlg.h
ImplementationFile=SUSANDlg.cpp

[CLS:CTHRESHDlg]
Type=0
BaseClass=CDialog
HeaderFile=THRESHDlg.h
ImplementationFile=THRESHDlg.cpp

[DLG:IDD_DIALOG_BANDSEL]
Type=1
Class=CBandSelDlg
ControlCount=16
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT_R,edit,1350631552
Control4=IDC_STATIC_R,static,1342308352
Control5=IDC_STATIC_G,static,1342308352
Control6=IDC_EDIT_G,edit,1350631552
Control7=IDC_STATIC_B,static,1342308352
Control8=IDC_EDIT_B,edit,1350631552
Control9=IDC_EDIT_BAND,edit,1350633600
Control10=IDC_STATIC_BANDS,static,1342308352
Control11=IDC_STATIC_NDVI,static,1342308352
Control12=IDC_EDIT_NDVI,edit,1350631552
Control13=IDC_STATIC_EB,static,1342308352
Control14=IDC_EDIT_EB,edit,1350631552
Control15=IDC_STATIC_TYPE,static,1342308352
Control16=IDC_EDIT_TYPE,edit,1350633600

[DLG:IDD_DIALOG_CANNY]
Type=1
Class=CCannyPara
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_HEADLINE,static,1342308352
Control4=IDC_EDIT_LOW,edit,1350631552
Control5=IDC_EDIT_HIGH,edit,1350631552
Control6=IDC_STATIC_LOW,static,1342308352
Control7=IDC_STATIC_HIGH,static,1342308352

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_DIALOG_INITIATE]
Type=1
Class=CInitiateDlg
ControlCount=23
Control1=IDC_RADIO_DM,button,1342308361
Control2=IDC_RADIO_HOMO,button,1342177289
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,static,1342308352
Control6=IDC_EDIT_K,edit,1350631552
Control7=IDC_EDIT_MINISIZE,edit,1350631552
Control8=IDC_STATIC_MINSIZE,static,1342308352
Control9=IDC_STATIC_LAMDA,static,1342308352
Control10=IDC_EDIT_LAMDA,edit,1350631552
Control11=IDC_STATIC_LN,static,1342308352
Control12=IDC_EDIT_LOOP,edit,1350631552
Control13=IDC_STATIC_Scale,static,1342308352
Control14=IDC_EDIT_SCALE,edit,1350631552
Control15=IDC_STATIC_SPEWEI,static,1342308352
Control16=IDC_STATIC_COMWEI,static,1342308352
Control17=IDC_EDIT_SPEWEI,edit,1350631552
Control18=IDC_EDIT_PACWEI,edit,1350631552
Control19=IDC_STATIC_OPT,static,1342308352
Control20=IDC_STATIC_LWA,static,1342308352
Control21=IDC_EDIT_LWARRAY,edit,1350631552
Control22=IDC_COMBO_METRIC,combobox,1344340995
Control23=IDC_STATIC_METRIC,static,1342308352

[DLG:IDD_DIALOG_FEATUREEXTR]
Type=1
Class=CMFeatDlg
ControlCount=12
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_MIND,static,1342308352
Control4=IDC_STATIC_MAXD,static,1342308352
Control5=IDC_STATIC_LEVEL,static,1342308352
Control6=IDC_EDIT_MINDIFF,edit,1350631552
Control7=IDC_EDIT_MAXDIFF,edit,1350631552
Control8=IDC_EDIT_LEVEL,edit,1350631552
Control9=IDC_EDIT_BANDWT,edit,1350631552
Control10=IDC_STATIC_BWA,static,1342308352
Control11=IDC_STATIC_LEVELUSE,static,1342308352
Control12=IDC_EDIT_LEVELUSE,edit,1350631552

[DLG:IDD_DIALOG_MORPH]
Type=1
Class=CMorphDlg
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_EBAND,static,1342308352
Control4=IDC_EDIT_EBAND,edit,1350631552
Control5=IDC_COMBO_MORPH,combobox,1344340995
Control6=IDC_STATIC_MT,static,1342308352

[DLG:IDD_DIALOG_MRFOPTIM]
Type=1
Class=CMRFOptimDlg
ControlCount=16
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_MRFCD,static,1342308352
Control4=IDC_COMBO_MRF,combobox,1344339970
Control5=IDC_STATIC_NUMCLS,static,1342308352
Control6=IDC_EDIT_NC,edit,1350631552
Control7=IDC_STATIC_BETA,static,1342308352
Control8=IDC_EDIT_BETA,edit,1350631552
Control9=IDC_STATIC_T,static,1342308352
Control10=IDC_EDIT_T,edit,1350631552
Control11=IDC_STATIC_T0,static,1342308352
Control12=IDC_EDIT_T0,edit,1350631552
Control13=IDC_STATIC_C,static,1342308352
Control14=IDC_EDIT_C,edit,1350631552
Control15=IDC_STATIC_ALPHA,static,1342308352
Control16=IDC_EDIT_ALPHA,edit,1350631552

[DLG:IDD_DIALOG_PREVIEW]
Type=1
Class=CPreviewDlg
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_SLIDER1,msctls_trackbar32,1342242825
Control4=IDC_STATIC_COUNT,static,1342308352
Control5=IDC_STATIC_THRESH,static,1342308352
Control6=IDC_STATIC_MIN,static,1342308352
Control7=IDC_STATIC_MAX,static,1342308352
Control8=IDC_STATIC_PICBOX,static,1342177287

[DLG:IDD_DIALOG_SEGOUT]
Type=1
Class=CResultDlg
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT_TIME,edit,1350633600
Control4=IDC_EDIT_COMPS,edit,1350633600
Control5=IDC_STATIC_TIME,static,1342308352
Control6=IDC_STATIC_BITNUM,static,1342308352
Control7=IDC_STATIC_QS,static,1342308352
Control8=IDC_EDIT_QS,edit,1350633600

[DLG:IDD_DIALOG_RESULTIMAGE]
Type=1
Class=CResultImageDlg
ControlCount=1
Control1=IDC_RESULTIMAGE,static,1342177287

[DLG:IDD_SEARCHPARA]
Type=1
Class=CSegmentPara

[DLG:IDD_COLOR_PALETTE]
Type=1
Class=CSegParaPopDlg

[DLG:IDD_DIALOG_SELPROP]
Type=1
Class=CSelPropDlg
ControlCount=12
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_COMBO_SELPROP,combobox,1344339970
Control4=IDC_STATIC_SELPROP,static,1342308352
Control5=IDC_EDIT_TOP,edit,1350631552
Control6=IDC_EDIT_BOT,edit,1350631552
Control7=IDC_EDIT_LEFT,edit,1350631552
Control8=IDC_EDIT_RIGHT,edit,1350631552
Control9=IDC_STATIC_TOP,static,1342308352
Control10=IDC_STATIC_BOTTOM,static,1342308352
Control11=IDC_STATIC_LEFT,static,1342308352
Control12=IDC_STATIC_RIGHT,static,1342308352

[DLG:IDD_DIALOG_SETTHRESH]
Type=1
Class=CSetPropDlg
ControlCount=12
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_AREA,static,1342308352
Control4=IDC_EDIT_MINAREA,edit,1350631552
Control5=IDC_STATIC_MEANW,static,1342308352
Control6=IDC_EDIT_MINWID,edit,1350631552
Control7=IDC_STATIC_BLWR,static,1342308352
Control8=IDC_EDIT_MAXLWR,edit,1350631552
Control9=IDC_STATIC_COMPCT,static,1342308352
Control10=IDC_EDIT_MINCMPCT,edit,1350631552
Control11=IDC_STATIC_MAXAREA,static,1342308352
Control12=IDC_EDIT_MAXAREA,edit,1350631552

[DLG:IDD_DIALOG_SUSAN]
Type=1
Class=CSUSANDlg
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_HEAD,static,1342308352
Control4=IDC_STATIC_BT,static,1342308352
Control5=IDC_STATIC_MASK,static,1342308352
Control6=IDC_EDIT_BT,edit,1350631552
Control7=IDC_RADIO_GAUSSIAN,button,1342308361
Control8=IDC_RADIO_3X3,button,1342177289

[DLG:IDD_DIALOG_CDTHRESH]
Type=1
Class=CTHRESHDlg
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_SLIDER,msctls_trackbar32,1342242840
Control4=IDC_STATIC_THRESH,static,1342308352
Control5=IDC_STATIC_MIN,static,1342308352
Control6=IDC_STATIC_MAX,static,1342308352
Control7=IDC_STATIC_COUNT,static,1342308352

[MNU:IDR_IMAGETYPE (English (U.S.))]
Type=1
Class=CImageDoc
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_PROCESS_HISTOEQUAL
Command16=ID_PREP_SOBEL
Command17=ID_PREP_CANNY
Command18=ID_PREP_SUSAN
Command19=ID_PROCESS_GUASS
Command20=ID_PREP_MORPH
Command21=ID_PREP_GABORTRANS
Command22=ID_PREP_ENTROPY
Command23=ID_PREP_OPENING
Command24=ID_PREP_SR
Command25=ID_PREP_SATHUE
Command26=ID_PREP_TEXTIMG
Command27=ID_CLASS_ERODE
Command28=ID_SEG_INITIATE
Command29=ID_SEG_QT
Command30=ID_SEG_REGMEG
Command31=ID_SEG_GS
Command32=ID_SEG_GSHC
Command33=ID_SEG_QTHC
Command34=ID_CLASS_PREVIEW
Command35=ID_SEG_RANDINDEX
Command36=ID_EVAL_QS
Command37=ID_SEG_SAVE
Command38=ID_SEG_SAVETOUR
Command39=ID_SEG_EXPORTSHP
Command40=ID_SEG_MEANSHIFTDZJ
Command41=ID_SEG_WATERSHEDV
Command42=ID_SEG_WATERSHEDG
Command43=ID_SEG_WATERSHEDSEQ
Command44=ID_SEG_PYRMEANSHIFT
Command45=ID_SEG_MEANSHIFTEDISON
Command46=ID_CLASS_MRFCD
Command47=ID_CLASS_COLORMRF
Command48=ID_CLASS_KMEANS
Command49=ID_CLASS_BUILDINGISODATA
Command50=ID_CLASS_BUILDING
Command51=ID_CLASS_ROADCLUMP
Command52=ID_CLASS_MULTIBUILD
Command53=ID_TEXTURE_ROSINCD
Command54=ID_TEXTURE_CORRBINARY
Command55=ID_TEXTURE_GRADCORR
Command56=ID_TEXTURE_HISTOSTATCD
Command57=ID_CLASS_MULTIFEATURE
Command58=ID_TEXTURE_GEOMETRICCD
Command59=ID_VIEW_TOOLBAR
Command60=ID_VIEW_STATUS_BAR
Command61=ID_VIEW_ZOOMIN
Command62=ID_VIEW_ZOOMOUT
Command63=ID_VIEW_ORIGIN
Command64=ID_VIEW_OUTCOME
Command65=ID_WINDOW_NEW
Command66=ID_WINDOW_CASCADE
Command67=ID_WINDOW_TILE_HORZ
Command68=ID_WINDOW_ARRANGE
Command69=ID_APP_ABOUT
CommandCount=69

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_VIEW_ORIGIN
CommandCount=9

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_DIALOG_IMAGEPROP]
Type=1
Class=?
ControlCount=14
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC_VAR,static,1342308352
Control4=IDC_EDIT_IMGATT,edit,1350631552
Control5=IDC_STATIC_NN,static,1342308352
Control6=IDC_EDIT_NN,edit,1350631552
Control7=IDC_STATIC_RZ,static,1342308352
Control8=IDC_EDIT_RS,edit,1350631552
Control9=IDC_STATIC_CX,static,1342308352
Control10=IDC_EDIT_CX,edit,1350631552
Control11=IDC_STATIC_CY,static,1342308352
Control12=IDC_EDIT_CY,edit,1350631552
Control13=IDC_STATIC_FR,static,1342308352
Control14=IDC_EDIT_FR,edit,1350631552

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_APP_ABOUT
CommandCount=8

