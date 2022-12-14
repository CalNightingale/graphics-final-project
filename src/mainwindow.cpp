#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <iostream>

void MainWindow::initialize() {
    realtime = new Realtime;

    QHBoxLayout *hLayout = new QHBoxLayout; // horizontal alignment
    QVBoxLayout *vLayout = new QVBoxLayout(); // vertical alignment
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addLayout(vLayout);
    hLayout->addWidget(realtime, 1);
    this->setLayout(hLayout);

    // Create labels in sidebox
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    QLabel *tesselation_label = new QLabel(); // Parameters label
    tesselation_label->setText("Tesselation");
    tesselation_label->setFont(font);
    QLabel *camera_label = new QLabel(); // Camera label
    camera_label->setText("Camera");
    camera_label->setFont(font);
    QLabel *filters_label = new QLabel(); // Filters label
    filters_label->setText("Filters");
    filters_label->setFont(font);
    QLabel *ec_label = new QLabel(); // Extra Credit label
    ec_label->setText("Extra Credit");
    ec_label->setFont(font);
    QLabel *param1_label = new QLabel(); // Parameter 1 label
    param1_label->setText("Seed:");
    QLabel *param2_label = new QLabel(); // Parameter 2 label
    param2_label->setText("Number of Biomes:");
    QLabel *param3_label = new QLabel(); // Parameter 2 label
    param3_label->setText("Max Height:");
    QLabel *param4_label = new QLabel(); // Parameter 2 label
    param4_label->setText("Size:");
    QLabel *param5_label = new QLabel(); // Parameter 2 label
    param5_label->setText("Time of Day:");
    QLabel *near_label = new QLabel(); // Near plane label
    near_label->setText("Near Plane:");
    QLabel *far_label = new QLabel(); // Far plane label
    far_label->setText("Far Plane:");

    QLabel *toonShading_label = new QLabel(); //  toon shading label.
    toonShading_label->setText("Toon Shading:");


    marvelCheck = new QCheckBox();
    marvelCheck->setText(QStringLiteral("Just marvel at the beauty:"));
    marvelCheck->setChecked(false);

    playerModeCheck = new QCheckBox();
    playerModeCheck->setText(QStringLiteral("Player Mode:"));
    playerModeCheck->setChecked(false);

    toonCheck = new QCheckBox();
    toonCheck->setText(QStringLiteral("Toon ON:"));
    toonCheck->setChecked(false);

    // Creates the boxes containing the parameter sliders and number boxes
    QGroupBox *p1Layout = new QGroupBox(); // horizonal slider 1 alignment
    QHBoxLayout *l1 = new QHBoxLayout();
    QGroupBox *p2Layout = new QGroupBox(); // horizonal slider 2 alignment
    QHBoxLayout *l2 = new QHBoxLayout();
    QGroupBox *p3Layout = new QGroupBox(); // horizonal slider 3 alignment
    QHBoxLayout *l3 = new QHBoxLayout();
    QGroupBox *p4Layout = new QGroupBox(); // horizonal slider 3 alignment
    QHBoxLayout *l4 = new QHBoxLayout();
    QGroupBox *p5Layout = new QGroupBox(); // horizonal slider 3 alignment
    QHBoxLayout *l5 = new QHBoxLayout();

    QGroupBox *p6Layout = new QGroupBox(); // cel shading alignment
    QHBoxLayout *l6 = new QHBoxLayout();

    // Create slider controls to control parameters
    p1Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 1 slider
    p1Slider->setTickInterval(1);
    p1Slider->setMinimum(0);
    p1Slider->setMaximum(1000);
    p1Slider->setValue(1);

    p1Box = new QSpinBox();
    p1Box->setMinimum(0);
    p1Box->setMaximum(1000);
    p1Box->setSingleStep(1);
    p1Box->setValue(0);

    p2Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 2 slider
    p2Slider->setTickInterval(1);
    p2Slider->setMinimum(1);
    p2Slider->setMaximum(50);
    p2Slider->setValue(1);

    p2Box = new QSpinBox();
    p2Box->setMinimum(1);
    p2Box->setMaximum(50);
    p2Box->setSingleStep(1);
    p2Box->setValue(1);

    p3Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 2 slider
    p3Slider->setTickInterval(10);
    p3Slider->setMinimum(10);
    p3Slider->setMaximum(500);
    p3Slider->setValue(100);

    p3Box = new QSpinBox();
    p3Box->setMinimum(10);
    p3Box->setMaximum(500);
    p3Box->setSingleStep(10);
    p3Box->setValue(100);

    p4Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 4 slider
    p4Slider->setTickInterval(32);
    p4Slider->setMinimum(64);
    p4Slider->setMaximum(256);
    p4Slider->setValue(128);

    p4Box = new QSpinBox();
    p4Box->setMinimum(64);
    p4Box->setMaximum(256);
    p4Box->setSingleStep(32);
    p4Box->setValue(128);

    p5Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 5 slider
    p5Slider->setTickInterval(1);
    p5Slider->setMinimum(1);
    p5Slider->setMaximum(5000);
    p5Slider->setValue(2500);

    p5Box = new QSpinBox();
    p5Box->setMinimum(1);
    p5Box->setMaximum(5000);
    p5Box->setSingleStep(1);
    p5Box->setValue(2500);

    p6Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 1 slider
    p6Slider->setTickInterval(1);
    p6Slider->setMinimum(1);
    p6Slider->setMaximum(100);
    p6Slider->setValue(1);

    p6Box = new QSpinBox();
    p6Box->setMinimum(1);
    p6Box->setMaximum(100);
    p6Box->setSingleStep(1);
    p6Box->setValue(1);

    // Adds the slider and number box to the parameter layouts
    l1->addWidget(p1Slider);
    l1->addWidget(p1Box);
    p1Layout->setLayout(l1);

    l2->addWidget(p2Slider);
    l2->addWidget(p2Box);
    p2Layout->setLayout(l2);

    l3->addWidget(p3Slider);
    l3->addWidget(p3Box);
    p3Layout->setLayout(l3);

    l4->addWidget(p4Slider);
    l4->addWidget(p4Box);
    p4Layout->setLayout(l4);

    l5->addWidget(p5Slider);
    l5->addWidget(p5Box);
    p5Layout->setLayout(l5);

    l6->addWidget(p6Slider);
    l6->addWidget(p6Box);
    p6Layout->setLayout(l6);

    vLayout->addWidget(param1_label);
    vLayout->addWidget(p1Layout);
    vLayout->addWidget(param2_label);
    vLayout->addWidget(p2Layout);
    vLayout->addWidget(param3_label);
    vLayout->addWidget(p3Layout);
    vLayout->addWidget(param5_label);
    vLayout->addWidget(p5Layout);
    vLayout->addWidget(toonShading_label);
    vLayout->addWidget(p6Layout);

    vLayout->addWidget(marvelCheck);
    vLayout->addWidget(playerModeCheck);
    vLayout->addWidget(toonCheck);

    connectUIElements();

    // Set default values of 5 for tesselation parameters
    onValChangeP1(0);
    onValChangeP2(1);
    onValChangeP3(100);
    onValChangeP5(2500);
    onValChangeToonSlider(3);

}

void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

void MainWindow::connectUIElements() {
    connectParam1();
    connectParam2();
    connectParam3();
    connectParam5();
    connectToon();
    connectMarvel();

    connectToonCheck();
    connectPlayerModeCheck();
}
void MainWindow::connectToonCheck() {
    connect(toonCheck, &QCheckBox::clicked, this, &MainWindow::onToonCheck);
}
void MainWindow::connectPlayerModeCheck() {
    connect(playerModeCheck, &QCheckBox::clicked, this, &MainWindow::onconnectPlayerModeCheck);
}
void MainWindow::connectMarvel() {
    connect(marvelCheck, &QCheckBox::clicked, this, &MainWindow::onMarvelCheck);
}

void MainWindow::connectPerPixelFilter() {
    connect(filter1, &QCheckBox::clicked, this, &MainWindow::onPerPixelFilter);
}

void MainWindow::connectKernelBasedFilter() {
    connect(filter2, &QCheckBox::clicked, this, &MainWindow::onKernelBasedFilter);
}

void MainWindow::connectUploadFile() {
    connect(uploadFile, &QPushButton::clicked, this, &MainWindow::onUploadFile);
}

void MainWindow::connectParam1() {
    connect(p1Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP1);
    connect(p1Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP1);
}

void MainWindow::connectParam2() {
    connect(p2Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP2);
    connect(p2Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP2);
}

void MainWindow::connectParam3() {
    connect(p3Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP3);
    connect(p3Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP3);
}

void MainWindow::connectParam4() {
    connect(p4Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP4);
    connect(p4Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP4);
}

void MainWindow::connectParam5() {
    connect(p5Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP5);
    connect(p5Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP5);
}

void MainWindow::connectNear() {
    connect(nearSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeNearSlider);
    connect(nearBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeNearBox);
}

void MainWindow::connectFar() {
    connect(farSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeFarSlider);
    connect(farBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeFarBox);
}

void MainWindow::connectExtraCredit() {
    connect(ec1, &QCheckBox::clicked, this, &MainWindow::onExtraCredit1);
    connect(ec2, &QCheckBox::clicked, this, &MainWindow::onExtraCredit2);
    connect(ec3, &QCheckBox::clicked, this, &MainWindow::onExtraCredit3);
    connect(ec4, &QCheckBox::clicked, this, &MainWindow::onExtraCredit4);
}

void MainWindow::connectToon() {
    connect(p6Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeToonSlider);
    connect(p6Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeToonSlider);
}

void MainWindow::onMarvelCheck() {
    settings.marvelCheck = !settings.marvelCheck;
    realtime->settingsChanged();
}
void MainWindow::onToonCheck() {
    settings.toonCheck = !settings.toonCheck;
    realtime->settingsChanged();
}
void MainWindow::onconnectPlayerModeCheck() {
    settings.playerCheck = !settings.playerCheck;
    realtime->settingsChanged();
    realtime->teleportToOrigin();
}

void MainWindow::onPerPixelFilter() {
    settings.perPixelFilter = !settings.perPixelFilter;
    realtime->settingsChanged();
}

void MainWindow::onKernelBasedFilter() {
    settings.kernelBasedFilter = !settings.kernelBasedFilter;
    realtime->settingsChanged();
}

void MainWindow::onUploadFile() {
    // Get abs path of scene file
    QString configFilePath = QFileDialog::getOpenFileName(this, tr("Upload File"), QDir::homePath(), tr("Scene Files (*.xml)"));
    if (configFilePath.isNull()) {
        std::cout << "Failed to load null scenefile." << std::endl;
        return;
    }

    settings.sceneFilePath = configFilePath.toStdString();

    std::cout << "Loaded scenefile: \"" << configFilePath.toStdString() << "\"." << std::endl;

    realtime->sceneChanged();
}


void MainWindow::onValChangeP1(int newValue) {
    p1Slider->setValue(newValue);
    p1Box->setValue(newValue);
    settings.shapeParameter1 = p1Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeP2(int newValue) {
    p2Slider->setValue(newValue);
    p2Box->setValue(newValue);
    settings.shapeParameter2 = p2Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeP3(int newValue) {
    p3Slider->setValue(newValue);
    p3Box->setValue(newValue);
    settings.shapeParameter3 = p3Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeP4(int newValue) {
    p4Slider->setValue(newValue);
    p4Box->setValue(newValue);
    settings.shapeParameter4 = p4Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeP5(int newValue) {
    p5Slider->setValue(newValue);
    p5Box->setValue(newValue);
    settings.shapeParameter5 = p5Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearSlider(int newValue) {
    //nearSlider->setValue(newValue);
    nearBox->setValue(newValue/100.f);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarSlider(int newValue) {
    //farSlider->setValue(newValue);
    farBox->setValue(newValue/100.f);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearBox(double newValue) {
    nearSlider->setValue(int(newValue*100.f));
    //nearBox->setValue(newValue);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarBox(double newValue) {
    farSlider->setValue(int(newValue*100.f));
    //farBox->setValue(newValue);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

// Extra Credit:

void MainWindow::onExtraCredit1() {
    settings.extraCredit1 = !settings.extraCredit1;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit2() {
    settings.extraCredit2 = !settings.extraCredit2;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit3() {
    settings.extraCredit3 = !settings.extraCredit3;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit4() {
    settings.extraCredit4 = !settings.extraCredit4;
    realtime->settingsChanged();
}



void MainWindow::onValChangeToonSlider(int newValue) {
    p6Slider->setValue(newValue);
    p6Box->setValue(newValue);
    settings.toonParam = p6Slider->value();
    realtime->settingsChanged();
}

