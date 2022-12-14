#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "realtime.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void connectUIElements();
    void connectParam1();
    void connectParam2();
    void connectParam3();
    void connectParam4();
    void connectParam5();
    void connectNear();
    void connectFar();
    void connectPerPixelFilter();
    void connectKernelBasedFilter();
    void connectUploadFile();
    void connectExtraCredit();

    void connectToon();

    Realtime *realtime;
    QCheckBox *filter1;
    QCheckBox *filter2;
    QPushButton *uploadFile;
    QSlider *p1Slider;
    QSlider *p2Slider;
    QSlider *p3Slider;
    QSlider *p4Slider;
    QSlider *p5Slider;
    QSpinBox *p1Box;
    QSpinBox *p2Box;
    QSpinBox *p3Box;
    QSpinBox *p4Box;
    QSpinBox *p5Box;
    QSlider *nearSlider;
    QSlider *farSlider;
    QDoubleSpinBox *nearBox;
    QDoubleSpinBox *farBox;

    QSpinBox *p6Box;
    QSlider *p6Slider;

    QCheckBox *marvelCheck;
    QCheckBox *playerModeCheck;
    QCheckBox *toonCheck;

    // Extra Credit:
    QCheckBox *ec1;
    QCheckBox *ec2;
    QCheckBox *ec3;
    QCheckBox *ec4;

private slots:
    void onPerPixelFilter();
    void onKernelBasedFilter();
    void onUploadFile();
    void onValChangeP1(int newValue);
    void onValChangeP2(int newValue);
    void onValChangeP3(int newValue);
    void onValChangeP4(int newValue);
    void onValChangeP5(int newValue);

    void connectMarvel();
    void connectToonCheck();

    void connectPlayerModeCheck();

    void onValChangeNearSlider(int newValue);
    void onValChangeFarSlider(int newValue);
    void onValChangeNearBox(double newValue);
    void onValChangeFarBox(double newValue);

    void onValChangeToonSlider(int newValue);

    void onMarvelCheck();
    void onToonCheck();
    void onconnectPlayerModeCheck();


    // Extra Credit:
    void onExtraCredit1();
    void onExtraCredit2();
    void onExtraCredit3();
    void onExtraCredit4();
};
