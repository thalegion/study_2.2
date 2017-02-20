
//====================================================================
//        THREADS.H

//=== FUNCTION PROTOTYPES ===

	// window procedures
LRESULT WINAPI Main_WndProc( HWND , UINT, WPARAM, LPARAM );
LRESULT WINAPI Thread_WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL	WINAPI About_DlgProc( HWND, UINT, WPARAM, LPARAM );

	// message handlers
BOOL Main_OnCreate( HWND, LPCREATESTRUCT );
void Main_OnInitMenu( HWND, HMENU );
void Main_OnCommand( HWND, int, HWND, UINT );
void Main_OnSize( HWND, UINT, int, int );
void Main_OnTimer( HWND, UINT );
void Main_OnDestroy( HWND );

	// other procedures
BOOL InitializeApp();
BOOL CreateWindows();
void ClearChildWindows();
void UpdateListBox();
void DoThread( int );
void DrawProc( DWORD );
LONG StartThread( LPVOID );
void MakeAbout( HWND );

//=== CONSTANTS ===

	// maximum string size
#define MAX_BUFFER		64
	// thread states
#define ACTIVE          10
#define SUSPENDED       20
	// timer ID
#define TIMER           10

//=== RESOURCE IDs ===

	// resource objects
#define MENU_MAIN       10
#define DLG_ABOUT       11
#define ICON_APP        12
	// menu IDs
#define IDM_ABOUT       13
#define IDM_EXIT        14
#define IDM_SUSPEND     15
#define IDM_RESUME      16
#define IDM_INCREASE    17
#define IDM_DECREASE    18
#define IDM_USEMUTEX    19

	// string IDs
#define IDS_APPNAME     20
#define IDS_TITLE       21
#define IDS_THREAD      22
