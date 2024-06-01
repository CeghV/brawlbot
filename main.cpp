#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include <windows.h>
#include <stdlib.h>
#include <fstream>
#pragma comment(lib,"glew32.lib")
HHOOK hHook=NULL;
const std::string classes[4]={"ally","enemy","player","progscreen"};
bool pressed[4]={false,false,false,false};
void pressrelease(WORD key) {
    INPUT input={0};
    input.type=INPUT_KEYBOARD;
    input.ki.wVk=key;
    input.ki.dwFlags=0;
    SendInput(1,&input,sizeof(INPUT));
    input.ki.dwFlags=KEYEVENTF_KEYUP;
    SendInput(1,&input,sizeof(INPUT));
}
void presskey(WORD key) {
    INPUT input={0};
    input.type=INPUT_KEYBOARD;
    input.ki.wVk=key;
    input.ki.dwFlags=0;
    SendInput(1,&input,sizeof(INPUT));
}
void releasekey(WORD key)   {
    INPUT input={0};
    input.type=INPUT_KEYBOARD;
    input.ki.wVk=key;
    input.ki.dwFlags=KEYEVENTF_KEYUP;
    SendInput(1,&input,sizeof(INPUT));
}
void quitall()  {
    if(hHook!=NULL) {
        cv::destroyAllWindows();
        UnhookWindowsHookEx(hHook);
        hHook=NULL;
    }
}
cv::Mat getwindow(HWND hwnd)    {
    HDC hdc=GetDC(hwnd);
    HDC memDC=CreateCompatibleDC(hdc);
    RECT rect;
    GetClientRect(hwnd,&rect);
    int width=rect.right-rect.left;
    int height=rect.bottom-rect.top;
    HBITMAP hBitmap=CreateCompatibleBitmap(hdc,width,height);
    SelectObject(memDC,hBitmap);
    BitBlt(memDC,0,0,width,height,hdc,0,0,SRCCOPY);
    BITMAPINFOHEADER bi;
    bi.biSize=sizeof(BITMAPINFOHEADER);
    bi.biWidth=width;
    bi.biHeight=-height;
    bi.biPlanes=1;
    bi.biBitCount=32;
    bi.biCompression=BI_RGB;
    bi.biSizeImage=0;
    bi.biXPelsPerMeter=0;
    bi.biYPelsPerMeter=0;
    bi.biClrUsed=0;
    bi.biClrImportant=0;
    cv::Mat mat(height,width,CV_8UC4);
    GetDIBits(memDC,hBitmap,0,height,mat.data,(BITMAPINFO*)&bi,DIB_RGB_COLORS);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(hwnd,hdc);
    return mat;
}
int getindex(std::vector<int32_t> vector,int32_t searchfor)   {
    int index=-1;
    for(int i=0;i<vector.size();i++) {
        if(vector.at(i)==searchfor)  {
            index=i;
            break;
        }
    }
    return index;
}
void moveto(cv::Point2d center,cv::Point2d player,cv::Mat mat)  {
    bool reals[4];
    if(center.x<player.x)   {
        cv::line(mat,player,player-cv::Point2d(10,0),cv::Scalar(255,255,0),3);
        releasekey(0x44);
        presskey(0x41);
        reals[3]=false;
        reals[1]=true;
        pressed[3]=false;
        pressed[1]=true;
    }
    if(center.x>player.x)   {
        cv::line(mat,player,player+cv::Point2d(10,0),cv::Scalar(255,255,0),3);
        releasekey(0x41);
        presskey(0x44);
        reals[1]=false;
        reals[3]=true;
        pressed[1]=false;
        pressed[3]=true;
    }
    if(center.y<player.y)   {
        cv::line(mat,player,player-cv::Point2d(0,10),cv::Scalar(255,255,0),3);
        releasekey(0x53);
        presskey(0x57);
        reals[2]=false;
        reals[0]=true;
        pressed[2]=false;
        pressed[0]=true;
    }
    if(center.y>player.y)   {
        cv::line(mat,player,player+cv::Point2d(0,10),cv::Scalar(255,255,0),3);
        releasekey(0x57);
        presskey(0x53);
        reals[0]=false;
        reals[2]=true;
        pressed[0]=false;
        pressed[2]=true;
    }
    for(int ji=0;ji<4;ji++)    {
        bool j=reals[ji];
        bool k=pressed[ji];
        if(!j&&k)   {
            switch(ji)  {
                case 0:
                    releasekey(0x57);
                break;
                case 1:
                    releasekey(0x41);
                break;
                case 2:
                    releasekey(0x53);
                break;
                case 3:
                    releasekey(0x44);
                break;
            }
            pressed[ji]=false;
        }
    }
}
std::vector<int> getindexvec(std::vector<int32_t> vector,int32_t searchfor)   {
    std::vector<int> indexes;
    for(int i=0;i<vector.size();i++) {
        if(vector.at(i)==searchfor)  {
            indexes.push_back(i);
        }
    }
    return indexes;
}
int roundtonum(double toround)   {
    int rounder=(toround/10);
    return floor(rounder*10);
}
LRESULT CALLBACK quitcallback(int nCode,WPARAM wParam,LPARAM lParam)    {
    if(nCode==HC_ACTION)    {
        KBDLLHOOKSTRUCT *pKeyboard=(KBDLLHOOKSTRUCT *)lParam;
        if(wParam==WM_KEYDOWN||wParam==WM_SYSKEYDOWN)   {
            if(pKeyboard->vkCode==0x58) {
                std::cout<<"x key detected, quitting..."<<"\n";
                quitall();
                exit(EXIT_SUCCESS);
            }
        }
    }
    return CallNextHookEx(hHook,nCode,wParam,lParam);
}
int main()  {
    SetProcessDPIAware();
    // hHook=SetWindowsHookEx(WH_KEYBOARD_LL,quitcallback,NULL,0);
    // if(hHook==NULL) {
    //     std::cout<<"failed to install quit key hook"<<"\n";
    //     return -1;
    // }
    HWND hwnd=FindWindow(NULL,"BlueStacks App Player 1");
    if(hwnd)    {
        SetForegroundWindow(hwnd);
    }   else{
        std::cout<<"failed to find bluestacks window 'BlueStacks App Player 1'"<<"\n";
        quitall();
        exit(EXIT_FAILURE);
    }
    std::cout<<"initialized fine"<<"\n";
    MSG msg;
    cv::namedWindow("brawl bot",cv::WINDOW_NORMAL);
    cv::dnn::Net net=cv::dnn::readNetFromONNX("best.onnx");
    net.enableWinograd(false);
    while(true) {
        cv::Mat mat=getwindow(hwnd);
        cv::cvtColor(mat,mat,cv::COLOR_BGRA2BGR);
        cv::resize(mat,mat,cv::Size(416,416));
        cv::Mat blob=cv::dnn::blobFromImage(mat,1/255.0f,cv::Size(416,416),cv::Scalar(),true,false);
        net.setInput(blob);
        std::vector<cv::Mat> outs;
        net.forward(outs,net.getUnconnectedOutLayersNames());
        cv::transposeND(outs[0],{0,2,1},outs[0]);
        std::vector<cv::Point2d> centers;
        std::vector<int32_t> groups;
        for(auto preds:outs)    {
            preds=preds.reshape(1,preds.size[1]);
            for(int i=0;i<preds.rows;i++)   {
                cv::Mat scores=preds.row(i).colRange(4,preds.cols);
                double conf;
                cv::Point maxLoc;
                cv::minMaxLoc(scores,0,&conf,0,&maxLoc);
                if(conf<0.75f)  {
                    continue;
                }
                float* det=preds.ptr<float>(i);
                double cx=det[0];
                double cy=det[1];
                double w=det[2];
                double h=det[3];
                groups.push_back(maxLoc.x);
                centers.push_back(cv::Point2d(cx,cy));
                cv::rectangle(mat,cv::Rect2d(cx-0.5*w,cy-0.5*h,w,h),cv::Scalar(0,63*maxLoc.x,0));
                cv::putText(mat,classes[maxLoc.x],cv::Point2d(cx-0.5*w,cy-0.5*h),cv::FONT_HERSHEY_SIMPLEX,0.5f,cv::Scalar(0,0,255));
            }
        }
        int progscreen=std::count(groups.begin(),groups.end(),3);
        int player=std::count(groups.begin(),groups.end(),2);
        int enemy=std::count(groups.begin(),groups.end(),1);
        int ally=std::count(groups.begin(),groups.end(),0);
        if(progscreen>0&&(ally==0&&enemy==0&&player==0))    {
            releasekey(0x57);
            releasekey(0x41);
            releasekey(0x53);
            releasekey(0x44);
            for(int i=0;i<5;i++)    {
                pressrelease(0x46);
            }
        }   else{
            if(player>0)    {
                if(ally>0)  {
                    int cindex=getindex(groups,0);
                    cv::Point2d center=centers.at(cindex);
                    int pindex=getindex(groups,2);
                    cv::Point2d player=centers.at(pindex);
                    center.x=roundtonum(center.x);
                    center.y=roundtonum(center.y);
                    player.x=roundtonum(player.x);
                    player.y=roundtonum(player.y);
                    moveto(center,player,mat);
                }   else    {
                    if(enemy>0) {
                        int cindex=getindex(groups,1);
                        cv::Point2d center=centers.at(cindex);
                        int pindex=getindex(groups,2);
                        cv::Point2d player=centers.at(pindex);
                        double dist=sqrt(pow((center.x-player.x),2)+pow((center.y-player.y),2));
                        if(dist<250)    {
                            cv::line(mat,player,center,cv::Scalar(0,255,255));
                            pressrelease(0xA0);
                        }
                        center.x=roundtonum(center.x);
                        center.y=roundtonum(center.y);
                        player.x=roundtonum(player.x);
                        player.y=roundtonum(player.y);
                        moveto(center,player,mat);
                    }
                }
                if(enemy>0) {
                    int pindex=getindex(groups,2);
                    cv::Point2d player=centers.at(pindex);
                    for(int inx:getindexvec(groups,1))  {
                        int cindex=inx;
                        cv::Point2d center=centers.at(cindex);
                        double dist=sqrt(pow((center.x-player.x),2)+pow((center.y-player.y),2));
                        cv::line(mat,player,center,cv::Scalar(0,0,255));
                        if(dist<70) {
                            cv::line(mat,player,center,cv::Scalar(255,0,0));
                            pressrelease(0x20);
                            break;
                        }
                    }
                }
            }
        }
        imshow("brawl bot",mat);
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))    {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(20);
    }
    std::cout<<"program quitting..."<<"\n";
    quitall();
    return 0;
}