#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include <QtWidgets\QListWidgetItem>
#include <Helpers\Path.h>
#include <QtWidgets\QGraphicsDropShadowEffect>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

private:
    Ui::MainWindowClass ui;

private slots:
	void changeSelectionMode(bool selecting);
	void changeSelectedItem(bool upButtonPressed);
};
