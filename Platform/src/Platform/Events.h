#pragma once
#include "Base/Base.h"

namespace PH::Platform {

#define PH_EVENT_TYPE_NULL 0
#define PH_EVENT_TYPE_KEY_PRESSED 1
#define PH_EVENT_TYPE_KEY_RELEASED 2
#define PH_EVENT_TYPE_MOUSEBUTTON_PRESSED 3
#define PH_EVENT_TYPE_MOUSEBUTTON_RELEASED 4
#define PH_EVENT_TYPE_CHAR 5
#define PH_EVENT_TYPE_MOUSE_MOVED 6
#define PH_EVENT_TYPE_WINDOW_RESIZED 7

	//all keyboard defines
#define PH_CANCEL 0x03 	        //Control-break processing
#define PH_MBUTTON 	0x04        //Middle mouse button (three-button mouse)
#define PH_XBUTTON1 	0x05    //X1 mouse button
#define PH_XBUTTON2 	0x06    //X2 mouse button
#define PH_BACK 	0x08        //BACKSPACE key
#define PH_TAB 	0x09 	        //TAB key
#define PH_CLEAR 	0x0C 	    //CLEAR key
#define PH_RETURN 	0x0D 	    //ENTER key
#define PH_SHIFT 	0x10 	    //SHIFT key
#define PH_CONTROL 	0x11 	    //CTRL key
#define PH_MENU 	0x12 	    //ALT key
#define PH_PAUSE 	0x13 	    //PAUSE key
#define PH_CAPITAL 	0x14 	    //CAPS LOCK key
#define PH_KANA 	0x15 	    //NE Kana mode
#define PH_HANGUEL 	0x15 	    //NE Hanguel mode (maintained for compatibility; use VK_HANGUL)
#define PH_HANGUL 	0x15 	    //NE Hangul mode
#define PH_NE_ON 	0x16 	    //NE On
#define PH_JUNJA 	0x17 	    //NE Junja mode
#define PH_FINAL 	0x18 	    //NE final mode
#define PH_HANJA 	0x19 	    //NE Hanja mode
#define PH_KANJI 	0x19 	    //NE Kanji mode
#define PH_NE_OFF 	0x1A 	    //NE Off
#define PH_ESCAPE 	0x1B 	    //ESC key
#define PH_CONVERT 	0x1C 	    //NE convert
#define PH_NONCONVERT 	0x1D 	//NE nonconvert
#define PH_ACCEPT 	0x1E 	    //NE accept
#define PH_MODECHANGE 	0x1F 	//NE mode change request
#define PH_SPACE 	0x20 	    //SPACEBAR
#define PH_PRIOR 	0x21 	    //PAGE UP key
#define PH_NEXT 	0x22 	    //PAGE DOWN key
#define PH_END 	0x23 	        //END key
#define PH_HOME 	0x24 	    //HOME key
#define PH_LEFT 	0x25 	    //LEFT ARROW key
#define PH_UP 	0x26 	        //UP ARROW key
#define PH_RIGHT 	0x27 	    //RIGHT ARROW key
#define PH_DOWN 	0x28 	    //DOWN ARROW key
#define PH_SELECT 	0x29 	    //SELECT key
#define PH_PRINT 	0x2A 	    //PRINT key
#define PH_EXECUTE 	0x2B 	    //EXECUTE key
#define PH_SNAPSHOT 	0x2C 	//PRINT SCREEN key
#define PH_INSERT 	0x2D 	    //INS key
#define PH_DELETE 	0x2E 	    //DEL key
#define PH_HELP 	0x2F 	    //HELP key
#define PH_KEY_0  	0x30 	            //0 key
#define PH_KEY_1 	0x31 	            //1 key
#define PH_KEY_2 	0x32 	            //2 key
#define PH_KEY_3 	0x33 	            //3 key
#define PH_KEY_4 	0x34 	            //4 key
#define PH_KEY_5 	0x35 	            //5 key
#define PH_KEY_6 	0x36 	            //6 key
#define PH_KEY_7 	0x37 	            //7 key
#define PH_KEY_8 	0x38 	            //8 key
#define PH_KEY_9 	0x39 	            //9 key
#define PH_KEY_A 	0x41 	            //A key
#define PH_KEY_B 	0x42 	            //B key
#define PH_KEY_C 	0x43 	            //C key
#define PH_KEY_D 	0x44 	            //D key
#define PH_KEY_E 	0x45 	            //E key
#define PH_KEY_F 	0x46 	            //F key
#define PH_KEY_G 	0x47 	            //G key
#define PH_KEY_H 	0x48 	            //H key
#define PH_KEY_I 	0x49 	            //I key
#define PH_KEY_J 	0x4A 	            //J key
#define PH_KEY_K 	0x4B 	            //K key
#define PH_KEY_L 	0x4C 	            //L key
#define PH_KEY_M 	0x4D 	            //M key
#define PH_KEY_N 	0x4E 	            //N key
#define PH_KEY_O 	0x4F 	            //O key
#define PH_KEY_P 	0x50 	            //P key
#define PH_KEY_Q 	0x51 	            //Q key
#define PH_KEY_R 	0x52 	            //R key
#define PH_KEY_S 	0x53 	            //S key
#define PH_KEY_T 	0x54 	            //T key
#define PH_KEY_U 	0x55 	            //U key
#define PH_KEY_V 	0x56 	            //V key
#define PH_KEY_W 	0x57 	            //W key
#define PH_KEY_X 	0x58 	            //X key
#define PH_KEY_Y 	0x59 	            //Y key
#define PH_KEY_Z 	0x5A 	            //Z key
#define PH_LWIN 	0x5B 	            //Left Windows key (Natural keyboard)
#define PH_RWIN 	0x5C 	            //Right Windows key (Natural keyboard)
#define PH_APPS 	0x5D 	            //Applications key (Natural keyboard)
#define PH_SLEEP 	0x5F 	            //Computer Sleep key
#define PH_NUMPAD0 	0x60 	            //Numeric keypad 0 key
#define PH_NUMPAD1 	0x61 	            //Numeric keypad 1 key
#define PH_NUMPAD2 	0x62 	            //Numeric keypad 2 key
#define PH_NUMPAD3 	0x63 	            //Numeric keypad 3 key
#define PH_NUMPAD4 	0x64 	            //Numeric keypad 4 key
#define PH_NUMPAD5 	0x65 	            //Numeric keypad 5 key
#define PH_NUMPAD6 	0x66 	            //Numeric keypad 6 key
#define PH_NUMPAD7 	0x67 	            //Numeric keypad 7 key
#define PH_NUMPAD8 	0x68 	            //Numeric keypad 8 key
#define PH_NUMPAD9 	0x69 	            //Numeric keypad 9 key
#define PH_MULTIPLY 	0x6A 	        //Multiply key
#define PH_ADD 	0x6B 	                //Add key
#define PH_SEPARATOR 	0x6C 	        //Separator key
#define PH_SUBTRACT 	0x6D 	        //Subtract key
#define PH_DECIMAL 	0x6E 	            //Decimal key
#define PH_DIVIDE 	0x6F 	            //Divide key
#define PH_F1 	0x70 	                //F1 key
#define PH_F2 	0x71 	                //F2 key
#define PH_F3 	0x72 	                //F3 key
#define PH_F4 	0x73 	                //F4 key
#define PH_F5 	0x74 	                //F5 key
#define PH_F6 	0x75 	                //F6 key
#define PH_F7 	0x76 	                //F7 key
#define PH_F8 	0x77 	                //F8 key
#define PH_F9 	0x78 	                //F9 key
#define PH_F10 	0x79 	                //F10 key
#define PH_F11 	0x7A 	                //F11 key
#define PH_F12 	0x7B 	                //F12 key
#define PH_F13 	0x7C 	                //F13 key
#define PH_F14 	0x7D 	                //F14 key
#define PH_F15 	0x7E 	                //F15 key
#define PH_F16 	0x7F 	                //F16 key
#define PH_F17 	0x80 	                //F17 key
#define PH_F18 	0x81 	                //F18 key
#define PH_F19 	0x82 	                //F19 key
#define PH_F20 	0x83 	                //F20 key
#define PH_F21 	0x84 	                //F21 key
#define PH_F22 	0x85 	                //F22 key
#define PH_F23 	0x86 	                //F23 key
#define PH_F24 	0x87 	                //F24 key
#define PH_NUMLOCK 	0x90 	            //NUM LOCK key
#define PH_SCROLL 	0x91 	            //SCROLL LOCK key
#define PH_LSHIFT 	0xA0 	            //Left SHIFT key
#define PH_RSHIFT 	0xA1 	            //Right SHIFT key
#define PH_LCONTROL 	0xA2 	        //Left CONTROL key
#define PH_RCONTROL 	0xA3 	        //Right CONTROL key
#define PH_LMENU 	0xA4 	            //Left MENU key
#define PH_RMENU 	0xA5 	            //Right MENU key
#define PH_BROWSER_BACK 	0xA6 	    //Browser Back key
#define PH_BROWSER_FORWARD 	0xA7 	    //Browser Forward key
#define PH_BROWSER_REFRESH 	0xA8 	    //Browser Refresh key
#define PH_BROWSER_STOP 	0xA9 	    //Browser Stop key
#define PH_BROWSER_SEARCH 	0xAA 	    //Browser Search key
#define PH_BROWSER_FAVORITES 	0xAB 	//Browser Favorites key
#define PH_BROWSER_HOME 	0xAC 	    //Browser Start and Home key
#define PH_VOLUME_MUTE 	0xAD 	        //Volume Mute key
#define PH_VOLUME_DOWN 	0xAE 	        //Volume Down key
#define PH_VOLUME_UP 	0xAF 	        //Volume Up key
#define PH_MEDIA_NEXT_TRACK 	0xB0 	//Next Track key
#define PH_MEDIA_PREV_TRACK 	0xB1 	//Previous Track key
#define PH_MEDIA_STOP 	0xB2 	        //Stop Media key
#define PH_MEDIA_PLAY_PAUSE 	0xB3 	//Play/Pause Media key
#define PH_LAUNCH_MAIL 	0xB4 	        //Start Mail key
#define PH_LAUNCH_MEDIA_SELECT 	0xB5 	//Select Media key
#define PH_LAUNCH_APP1 	0xB6 	        //Start Application 1 key
#define PH_LAUNCH_APP2 	0xB7 	        //Start Application 2 key
#define PH_OEM_1 	0xBA 	            //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
#define PH_OEM_PLUS 	0xBB 	        //For any country/region, the '+' key
#define PH_OEM_COMMA 	0xBC 	        //For any country/region, the ',' key
#define PH_OEM_MINUS 	0xBD 	        //For any country/region, the '-' key
#define PH_OEM_PERIOD 	0xBE 	        //For any country/region, the '.' key
#define PH_OEM_2 	0xBF 	            //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
#define PH_OEM_3 	0xC0 	            //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
#define PH_OEM_4 	0xDB 	            //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
#define PH_OEM_5 	0xDC 	            //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
#define PH_OEM_6 	0xDD 	            //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
#define PH_OEM_7 	0xDE 	            //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
#define PH_OEM_8 	0xDF 	            //Used for miscellaneous characters; it can vary by keyboard.
#define PH_OEM_102 	0xE2 	            //The <> keys on the US standard keyboard, or the \\| key on the non-US 102-key keyboard
		
//mouse PHttons
#define PH_LMBUTTON 1
#define PH_RMBUTTON 2
#define PH_MMBUTTON 3

	

	/// <summary>
	/// KEY_DOWN: lparem(keycode)
	/// KEY_UP: lparam(keycode)
	struct Event {

		uint32 type;
		uint64 lparam;
		uint64 rparam;
		uint32 sender;

	};

}