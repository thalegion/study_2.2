//====================================================================
//	THREADS.C
//
//		Demonstrates Window NT's multithreading capabilities.
//
//	FUNCTIONS
//		WinMain             main message loop
//		InitializeApp       register windows, initialize variables
//		CreateWindows       create all the windows (6 of them)
//		Main_WndProc        process messages for main window
//		Main_OnCreate       //
//		Main_OnTimer        //      message handlers
//		Main_OnSize         //
//		Main_OnInitMenu     //
//		Main_OnCommand      //
//		Main_OnDestroy      //
//		DoThread            modify a thread according to user's command
//
//		ClearChildWindows   erase all shapes in all child windows
//		StartThread         the function the secondary threads execute
//		DrawProc            draw shapes in a child window
//		Thread_WndProc      process messages for a child window
//
//		MakeAbout           show the About box
//		About_DlgProc       process messages for the About box
//		About_OnInitDlg
//		About_OnCommand
//
//====================================================================

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>         // included for rand() and srand()
#include <time.h>
#include "threads.h"

//===================================================================
//      GLOBAL VARIABLES

HWND		hwndChild[4];       // windows where threads draw
HANDLE		hThread[4];         // handles for the four threads
HWND		hwndParent;         // main window
HWND		hwndList;           // list box window
HINSTANCE	hInst;              // current instance
int			iRandSeed;
DWORD		dwThreadID[4];      // array of thread ids
DWORD		dwThreadData[4];    // total shapes drawn by each thread
int			iRectCount[4];      // cumulative totals of shapes drawn
int			iState[4];          // current state of each thread
BOOL		bTerminate = FALSE; // TRUE to quit
BOOL		bUseMutex = FALSE;  // TRUE to make one thread draw at a time
HANDLE		hDrawMutex;         // mutex synchronization object

//===================================================================
//		WIN MAIN
//		Calls initializing procedures and runs the message loop

int WINAPI WinMain ( HINSTANCE hinstThis, HINSTANCE hinstPrev,
					 LPSTR lpszCmdLine,   int iCmdShow )
{
    MSG msg;

    hInst = hinstThis;          // store in global variable
    if( ! InitializeApp() )     // if application not initialized, exit here
        return( 0 );
	iRandSeed  = (int) time( NULL );
    ShowWindow( hwndParent, iCmdShow );
    UpdateWindow( hwndParent );
		// receive and forward messages from our queue
    while (GetMessage( &msg, NULL, 0, 0 ))
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    return( msg.wParam );
}

//===================================================================
//		INITIALIZE APP
//		Register two window classes and the create the windows

BOOL InitializeApp ( void )
{
    WNDCLASSEX wc;
	DWORD	dwError;
    char	szAppName[MAX_BUFFER] = "Threads";
    char	szTitle[MAX_BUFFER];
    char	szThread[MAX_BUFFER];
    int		iCount;

		// load strings from the string table
    LoadString( hInst, IDS_APPNAME, szAppName,  sizeof(szAppName) );
    LoadString( hInst, IDS_TITLE,   szTitle,    sizeof(szTitle)   );
    LoadString( hInst, IDS_THREAD,  szThread,   sizeof(szThread)  );
		// seed the random number generator 
	srand( iRandSeed++ );
	wc.cbSize = sizeof( WNDCLASSEX ); 
		// register the main window class
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = Main_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIcon( hInst, MAKEINTRESOURCE(ICON_APP) );
    wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = szAppName;
    wc.lpszClassName    = szAppName;
	wc.hIconSm			= NULL;
    if( ! RegisterClassEx( &wc ) )
	{
		dwError = GetLastError();
		return FALSE;
	}
		// register the class for child windows where threads draw
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = (WNDPROC) Thread_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground    = GetStockBrush( WHITE_BRUSH );
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = "ThreadClass";
	wc.hIconSm			= NULL;
    if( ! RegisterClassEx( &wc ) )
	{
		dwError = GetLastError();
		return FALSE;
	}
	    // Mark the initial state of each thread as SUSPENDED.
		// That is how they will be created.
    for( iCount = 0; iCount < 4; iCount++ ) iState[iCount] = SUSPENDED;
		// make the primary thread more important to facilitate user i/o
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
		// create all the windows
    return( CreateWindows() );
}

//===================================================================
//		CREATE WINDOWS
//		Create the parent window, the list box window, and the four
//		child windows.

BOOL CreateWindows ( void )
{
    DWORD	dwError;
	char	szAppName[MAX_BUFFER];
    char	szTitle[MAX_BUFFER];
    char	szThread[MAX_BUFFER];
    HMENU	hMenu;
    int		iCount;

		// load the relevant strings
    LoadString( hInst, IDS_APPNAME, szAppName,  sizeof(szAppName) );
    LoadString( hInst, IDS_TITLE,   szTitle,    sizeof(szTitle)   );
    LoadString( hInst, IDS_THREAD,  szThread,   sizeof(szThread)  );
	    // Create the main windows.  During CreateWindow the system
		// calls Main_OnCreate and initializes the threads.
    hMenu = LoadMenu( hInst, MAKEINTRESOURCE(MENU_MAIN) );
    hwndParent = CreateWindow( szAppName, szTitle,
							   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, hMenu, hInst, NULL );
    if( ! hwndParent )
    {
		dwError = GetLastError();	// for debug purposes
        return( FALSE );
    }
		// create the list box
    hwndList = CreateWindow( "LISTBOX", NULL,
							 WS_BORDER | WS_CHILD | WS_VISIBLE |
							 LBS_STANDARD | LBS_NOINTEGRALHEIGHT,
							 0, 0, 0, 0, hwndParent, (HMENU)1, hInst, NULL );
    if( ! hwndList )
    {
		dwError = GetLastError();	// for debug purposes
        return( FALSE );
    }
		// create the four child windows 
    for (iCount = 0; iCount < 4; iCount++)
    {
        hwndChild[iCount] = CreateWindow( "ThreadClass", NULL,
										  WS_BORDER | WS_CHILD |
										  WS_VISIBLE | WS_CLIPCHILDREN,
										  0, 0, 0, 0, hwndParent, 
										  NULL, hInst, NULL );
        if( ! hwndChild )
        {
			dwError = GetLastError();	// for debug purposes
		    return( FALSE );
        }
    }
    return( TRUE );
}

//===================================================================
//		MAIN_WNDPROC
//		All messages for the main window are processed here.

LRESULT WINAPI Main_WndProc( HWND hWnd,     UINT uMessage,
							 WPARAM wParam, LPARAM lParam )
{
    switch (uMessage)
    {
        HANDLE_MSG( hWnd, WM_CREATE, Main_OnCreate );		// initialize the threads 
        HANDLE_MSG( hWnd, WM_SIZE, Main_OnSize );			// reposition the child windows
        HANDLE_MSG( hWnd, WM_TIMER, Main_OnTimer );			// update the list box info
        HANDLE_MSG( hWnd, WM_INITMENU, Main_OnInitMenu );	// check or uncheck Use Mutex menu item
        HANDLE_MSG( hWnd, WM_COMMAND, Main_OnCommand );		// process commands from the menu
        HANDLE_MSG( hWnd, WM_DESTROY, Main_OnDestroy );		// clean up and close down
        default:
            return( DefWindowProc(hWnd, uMessage, wParam, lParam) );
    }
    return( 0L );
}

//===================================================================
//		MAIN_ONCREATE
//		Create the four threads and set the timer

BOOL Main_OnCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
    DWORD	dwError;
    UINT	uRet;
    int		iCount;

		// create the four threads, initially suspended
    for( iCount = 0; iCount < 4; iCount++ )
    {
        iRectCount[iCount] = 0;
        dwThreadData[iCount] = iCount;
        hThread[iCount] = CreateThread( NULL, 0,
										(LPTHREAD_START_ROUTINE)StartThread,
										(LPVOID)(&(dwThreadData[iCount])),
							            CREATE_SUSPENDED,
										(LPDWORD)(&(dwThreadID[iCount])) );
        if( ! hThread[iCount] )			// was the thread created?
        {
			dwError = GetLastError();	// for debug purposes
            return( FALSE );
        }
    }
	    // Create a timer with a one-second period.
		// The timer is used to update the list box.
    uRet = SetTimer( hWnd, TIMER, 1000, NULL );
    if( ! uRet )
    {                               // unable to create the timer
		dwError = GetLastError();	// for debug purposes
        return( FALSE );
    }
		// create a mutex synchronization object
    hDrawMutex = CreateMutex( NULL, FALSE, NULL );
    if( ! hDrawMutex )
    {                               // unable to create mutex
		dwError = GetLastError();	// for debug purposes
        KillTimer( hWnd, TIMER );   // stop the timer
        return( FALSE );
    }
		// start the threads with a priority below normal 
    for( iCount=0; iCount<4; iCount++ )
    {
        SetThreadPriority( hThread[iCount], THREAD_PRIORITY_BELOW_NORMAL );
        iState[iCount] = ACTIVE;
        ResumeThread( hThread[iCount] );
    }
    return( TRUE );     // Now all four threads are running!
}

//===================================================================
//		MAIN_ONSIZE
//		Position the list box and the four child windows.

void Main_OnSize( HWND hWnd, UINT uState, int cxClient, int cyClient )
{
    char  *	szText = "No Thread Data";
    int		iCount;

		// Suspend all active threads while the windows
		// resize and repaint themselves.  This pause
		// enables the screen to update more quickly.
    for (iCount = 0; iCount < 4; iCount++)
        if( iState[iCount] == ACTIVE )
            SuspendThread( hThread[iCount] );
		// place the list box across the top fourth of the window
    MoveWindow( hwndList, 0, 0, cxClient, cyClient / 4, TRUE );
		// Spread the 4 child windows across the bottom 3/4 of the
		// window. (The left border of the first one should be 0.)
    MoveWindow( hwndChild[0], 0, cyClient / 4 - 1, cxClient / 4 + 1,
                cyClient, TRUE);

    for (iCount = 1; iCount < 4; iCount++)
        MoveWindow( hwndChild[iCount], (iCount * cxClient) / 4,
                    cyClient / 4 - 1,  cxClient / 4 + 1, cyClient, TRUE);
	    // Add the default strings to the list box, and initialize
	    // the number of figures drawn to zero
    for( iCount = 0; iCount < 4; iCount++ )
    {
        iRectCount[iCount] = 0;
        ListBox_AddString( hwndList, szText );
    }
    ListBox_SetCurSel( hwndList, 0 );
		// reactivate the threads that were suspended while redrawing
    for( iCount = 0; iCount < 4; iCount++ )
        if( iState[iCount] == ACTIVE )
            ResumeThread( hThread[iCount] );
    return;
}

//===================================================================
//      MAIN_ONTIMER
//      Process the timer message

void Main_OnTimer ( HWND hWnd, UINT uTimerID )
{
    UpdateListBox();	// update the data shown in the listbox
    return;
}

//===================================================================
//		MAIN_ONINITMENU
//		Check or uncheck the Use Mutex menu item based on the
//		value of bUseMutex

void Main_OnInitMenu( HWND hWnd, HMENU hMenu )
{
    CheckMenuItem( hMenu, IDM_USEMUTEX,
                   MF_BYCOMMAND |
                   (UINT)(bUseMutex ? MF_CHECKED : MF_UNCHECKED) );
    return;
}

//===================================================================
//		MAIN_ONCOMMAND
//		Respond to commands from the user

void Main_OnCommand( HWND hWnd, int iCmd, HWND hwndCtl, UINT uCode )
{
    switch (iCmd)
    {
        case IDM_ABOUT:			// display the about box
            MakeAbout( hWnd );
            break;

        case IDM_EXIT:			// exit this program
            DestroyWindow( hWnd );
            break;

        case IDM_INCREASE:		// modify the priority or 
        case IDM_DECREASE:		// state of one of the threads
        case IDM_SUSPEND:
        case IDM_RESUME:
			DoThread( iCmd );	// adjust a thread
            break;
				
        case IDM_USEMUTEX:		// toggle the use of the mutex
            ClearChildWindows();
			bUseMutex = !bUseMutex;
            break;

        default:
            break;
    }
    return;
}

//===================================================================
//		MAIN_ONDESTROY
//		Kill the timer, set the terminate flag to zero, and post the
//		WM_QUIT message.  We're not closing the thread and mutex
//		handles in order to make the point that the system will
//		do it anyway when the process ends.  The threads may not
//		have time to see the bTerminate flag before the program
//		ends, but the final ExitProcess() will kill them anyway.

void Main_OnDestroy( HWND hWnd )
{
    bTerminate = TRUE;          // threads may not have time to see this
    KillTimer( hWnd, TIMER );   // destroy timer
    PostQuitMessage( 0 );       // post WM_QUIT to close app
    return;
}

//===================================================================
//      DO THREAD
//      Modify a thread's priority or change its state

void DoThread ( int iCmd )
{
    int iThread;
    int iPriority;

    iThread = ListBox_GetCurSel( hwndList );	// determine which thread to modify
    switch( iCmd )
    {
        case IDM_SUSPEND:		// if the thread is not suspended, then suspend it
            if( iState[iThread] != SUSPENDED )
            {
                SuspendThread( hThread[iThread] );
                iState[iThread] = SUSPENDED;
            }
            break;

        case IDM_RESUME:		// if the thread is not active, then activate it
            if( iState[iThread] != ACTIVE )
            {
                ResumeThread( hThread[iThread] );
                iState[iThread] = ACTIVE;
            }
            break;

        case IDM_INCREASE:		// Increase the thread's priority (unless
								// already at the highest level.)
            iPriority = GetThreadPriority( hThread[iThread] );
            switch( iPriority )
            {
                case THREAD_PRIORITY_LOWEST:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_BELOW_NORMAL );
                    break;

                case THREAD_PRIORITY_BELOW_NORMAL:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_NORMAL );
                    break;

                case THREAD_PRIORITY_NORMAL:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_ABOVE_NORMAL );
                    break;

                case THREAD_PRIORITY_ABOVE_NORMAL:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_HIGHEST );
                    break;

                default:
                    break;
            }
            break;

        case IDM_DECREASE:		// Decrease the thread's priority (unless
								// already at the lowest level.)
            iPriority = GetThreadPriority( hThread[iThread] );

            switch( iPriority )
            {
                case THREAD_PRIORITY_BELOW_NORMAL:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_LOWEST );
                    break;

                case THREAD_PRIORITY_NORMAL:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_BELOW_NORMAL );
                    break;

                case THREAD_PRIORITY_ABOVE_NORMAL:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_NORMAL );
                    break;

                case THREAD_PRIORITY_HIGHEST:
                    SetThreadPriority( hThread[iThread], THREAD_PRIORITY_ABOVE_NORMAL );
                    break;

                default:
                    break;
            }
            break;

/*
Priority						Meaning 
THREAD_PRIORITY_TIME_CRITICAL	Indicates a base-priority level of 15 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base-priority level of 31 for REALTIME_PRIORITY_CLASS processes. 
THREAD_PRIORITY_HIGHEST			Indicates 2 points above normal priority for the priority class. 
THREAD_PRIORITY_ABOVE_NORMAL	Indicates 1 point above normal priority for the priority class. 
THREAD_PRIORITY_NORMAL			Indicates normal priority for the priority class. 
THREAD_PRIORITY_BELOW_NORMAL	Indicates 1 point below normal priority for the priority class. 
THREAD_PRIORITY_LOWEST			Indicates 2 points below normal priority for the priority class. 
THREAD_PRIORITY_IDLE			Indicates a base-priority level of 1 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base-priority level of 16 for REALTIME_PRIORITY_CLASS processes. 
*/ 
        default:
            break;
    }
    return;
}

//===================================================================
//      CLEAR CHILD WINDOWS
//      Paint all four child windows white

void ClearChildWindows ( void )
{
    int iCount;

    for (iCount=0; iCount<4; iCount++)
    {
        RECT rcClient;          // client area
        HDC hDC;                // dc for child window
        BOOL bError;            // for testing return value
	
			// get the window's dimensions
        bError = GetClientRect( hwndChild[iCount], &rcClient );
        if( ! bError )	return;

        hDC = GetDC( hwndChild[iCount] );
        if( ! hDC )		return;

        SelectBrush( hDC, GetStockBrush(WHITE_BRUSH) );
        PatBlt( hDC, (int)rcClient.left, (int)rcClient.top,
                     (int)rcClient.right, (int)rcClient.bottom, PATCOPY );

        ReleaseDC( hwndChild[iCount], hDC );
    }
    return;
}

//===================================================================
//      UPDATE LIST BOX
//      Show new values in the list box window

void UpdateListBox ( void )
{
    char	szText[128];	// buffer for a list box string
    int		iPriority;		// one thread's current priority level
    int		iCount;			// control variable
    int		iSel;			// position of currently selected list box item

    iSel = ListBox_GetCurSel( hwndList );		// save current selection ID
    ListBox_ResetContent( hwndList );			// empty the list box
    for( iCount = 0; iCount < 4; iCount++ )		// add updated thread data
    {
		iPriority = GetThreadPriority( hThread[iCount] );
        wsprintf( szText,						// format the thread information
				  "Window ID: %d    Priority: %d    Status: %s    "
				  "Thread ID: 0x%04X:%04X    Figures Drawn: %d",
				  iCount, iPriority, 
				  ( iState[iCount] == ACTIVE ) ? "Active" : "Suspended",
				  HIWORD( dwThreadID[iCount] ), LOWORD( dwThreadID[iCount] ),
				  iRectCount[iCount]  );
		ListBox_AddString( hwndList, szText );	// add the text to the listbox
    }
    ListBox_SetCurSel( hwndList, iSel );	// restore the list box selection
    return;
}

//===================================================================
//      START THREAD
//      This is called when each thread begins execution.  The
//      threads enter a loop and continue drawing until a Boolean
//      flag turns TRUE (or the process ends).

LONG StartThread ( LPVOID lpThreadData )
{
    DWORD *pdwThreadID;     // pointer to a DWORD for storing thread's id
    DWORD dwWait;           // return value from WaitSingleObject

    pdwThreadID = lpThreadData;	// retrieve the thread's id
    while( ! bTerminate )		// continue until bTerminate == TRUE
    {
        if( bUseMutex )
        {						// draw when this thread gets the mutex
            dwWait = WaitForSingleObject( hDrawMutex, INFINITE );
            if( dwWait == 0 )
            {
                DrawProc( *pdwThreadID );
				Sleep(100);
                ReleaseMutex( hDrawMutex );
            }
        }
        else					// not using mutex; let the thread draw
            DrawProc( *pdwThreadID );
    }
    return( 0L );				// implicitly calls ExitThread()
}

//===================================================================
//        DRAW PROC
//        Draw five random rectangles or ellipses

void DrawProc ( DWORD dwID )
{
    HBRUSH hBrush;      // brush of a random color
    HBRUSH hbrOld;      // previously selected brush
    RECT rcClient;      // bounds of child window's client area
    HDC hDC;            // device context for child window
    LONG cxClient;      // client area dimensions (RECT members are LONG)
    LONG cyClient;
    int iCount;         // loop control variable
    int xStart;         // randomly generated coordinates
    int xStop;
    int yStart;
    int yStop;
    int iRed;           // randomly generated color values
    int iGreen;
    int iBlue;
    int iTotal;         // number of shapes to draw at a time
    BOOL bError;        // return value to test for error

    if( bUseMutex )
        iTotal = 50;    // If only one thread draws at a time,
						// let it draw more shapes at once.
    else
        iTotal = 1;

	srand( iRandSeed++ );	// re-seed the random generator
		// get the window's dimensions
    bError = GetClientRect( hwndChild[dwID], &rcClient );
    if( ! bError ) return;
    cxClient = rcClient.right - rcClient.left;
    cyClient = rcClient.bottom - rcClient.top;
		// do not draw if the window does not have any dimensions
    if( ( ! cxClient ) || ( ! cyClient ) ) return;
		// get an HDC to perform the drawing
    hDC = GetDC( hwndChild[dwID] );
    if( hDC )
    {		// draw the five random figures
        for( iCount = 0; iCount < iTotal; iCount++ )
        {
            iRectCount[dwID]++;
				// set the coordinates
            xStart	= (int)( rand() % cxClient );
            xStop	= (int)( rand() % cxClient );
            yStart	= (int)( rand() % cyClient );
            yStop	= (int)( rand() % cyClient );
				// set the color
            iRed	= rand() & 255;
            iGreen	= rand() & 255;
            iBlue	= rand() & 255;
				// create the solid brush
            hBrush = CreateSolidBrush(
						GetNearestColor( hDC, RGB( iRed, iGreen, iBlue ) ) );
            hbrOld = SelectBrush( hDC, hBrush );
				// draw a rectangle
            Rectangle( hDC,
					   min( xStart, xStop ), max( xStart, xStop ),
					   min( yStart, yStop ), max( yStart, yStop ) );
				// delete the brush 
            DeleteBrush( SelectBrush( hDC, hbrOld ) );
			Sleep(100);
        }

	        // If only one thread is drawing at a time, clear
		    // the child window before the next thread draws
        if( bUseMutex )
        {
            SelectBrush( hDC, GetStockBrush(WHITE_BRUSH) );
            PatBlt( hDC, (int)rcClient.left, (int)rcClient.top,
                         (int)rcClient.right, (int)rcClient.bottom, PATCOPY );
        }
			// release the HDC
        ReleaseDC( hwndChild[dwID], hDC );
    }
    return;
}

//===================================================================
//      THREAD_WNDPROC
//      Process message sent to the threads window.  In this case no
//      messages are processed

LRESULT WINAPI Thread_WndProc( HWND hWnd,     UINT uMessage, 
							   WPARAM wParam, LPARAM lParam )
{
    switch( uMessage )
    {
        default:
            return( DefWindowProc( hWnd, uMessage, wParam, lParam ) );
    }
}

//===================================================================
//      MAKE ABOUT
//      Process requests to show the About box here

void MakeAbout( HWND hWnd )
{
    DialogBox( hInst, MAKEINTRESOURCE(DLG_ABOUT), hWnd, About_DlgProc );
    return;
}

//===================================================================
//      ABOUT_DLGPROC
//      Process messages for the About box window here

BOOL WINAPI About_DlgProc( HWND hDlg,	  UINT uMessage,
						   WPARAM wParam, LPARAM lParam )
{
    switch( uMessage )
    {
        case WM_INITDIALOG:
            return( TRUE );

        case WM_COMMAND:
            EndDialog( hDlg, TRUE );
            return( TRUE );

        default:
            return( FALSE );
    }
}
