#include <Windows.h>
#include <iostream>
#include <thread>
#include <random>
#include <atomic>

using namespace std;

atomic<bool> isSimulatingClick (false);
atomic<bool> isLeftClickActive (false);
atomic<bool> isRightClickActive(false);
atomic<int>  numExtraClicks(2); //defaults to 2

inline int AntiDetectionSolution(){
    static default_random_engine generator;
    static normal_distribution<double> distribution(35.0, 10.0);
    int delay = static_cast<int>(distribution(generator));
    return max(10, min(60, delay));
}

void SimulateClick(DWORD clickFlagDown, DWORD clickFlagUp){
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = clickFlagDown;
    SendInput(1, &input, sizeof(INPUT));

    this_thread::sleep_for(chrono::milliseconds(rand() % 10 + 10));

    input.mi.dwFlags = clickFlagUp;
    SendInput(1, &input, sizeof(INPUT));
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode >= 0 && wParam == WM_LBUTTONDOWN && isLeftClickActive){
        MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;

        if (!isSimulatingClick && mouseStruct != nullptr){
            cout<<"Left click detected! Simulating..."<<std::endl;

            thread([]() {
                isSimulatingClick = true;

                for (int i = 0; i < numExtraClicks; ++i){
                    this_thread::sleep_for(chrono::milliseconds(AntiDetectionSolution()));
                    SimulateClick(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
                }

                cout<<"Left clicks simulated."<<std::endl;
                isSimulatingClick = false;
            }).detach();
        }
    }

    if (nCode >= 0 && wParam == WM_RBUTTONDOWN && isRightClickActive) {
        MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;

        if (!isSimulatingClick && mouseStruct != nullptr){
            cout<<"Right click detected! Simulating..."<<endl;

            thread([](){
                isSimulatingClick = true;

                for (int i = 0; i < numExtraClicks; ++i){
                    this_thread::sleep_for(chrono::milliseconds(AntiDetectionSolution()));
                    SimulateClick(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
                }

                cout<<"Right clicks simulated."<<endl;
                isSimulatingClick = false;
            }).detach();
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    int userChoice = 0;
    while(userChoice < 1 || userChoice > 2){
        cout<<"how many extra clicks?(1 or 2): ";
        cin>>userChoice;
        if(userChoice >= 1 && userChoice <= 2){
            numExtraClicks = userChoice;
        }else {
            cout << "Invalid. choose between 1 and 2." << endl;
        }
    }

    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
    if (mouseHook == NULL) {
        cerr << "Failed to hook!" << endl;
        return 1;
    }

    cout << "Hooked..." << endl;

    thread([]() {
        while (true) {
            if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000){
                isLeftClickActive = !isLeftClickActive;
                cout << (isLeftClickActive ? "Left click simulation ON" : "Left click simulation OFF")<< endl;
                this_thread::sleep_for(chrono::milliseconds(300));
            }

            if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000){
                isRightClickActive = !isRightClickActive;
                cout<<(isRightClickActive ? "Right click simulation ON" : "Right click simulation OFF")<< endl;
                this_thread::sleep_for(chrono::milliseconds(300));
            }

            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }).detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(mouseHook);
    return 0;
}