/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include "CoverGlWidget.hpp"

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *actionSave_Texture;
    QAction *actionLoad_Texture;
    QAction *actionQuit;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    CoverGlWidget *cover_widget;
    QListWidget *cover_list;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(800, 600);
        actionSave_Texture = new QAction(MainWindowClass);
        actionSave_Texture->setObjectName(QStringLiteral("actionSave_Texture"));
        actionLoad_Texture = new QAction(MainWindowClass);
        actionLoad_Texture->setObjectName(QStringLiteral("actionLoad_Texture"));
        actionQuit = new QAction(MainWindowClass);
        actionQuit->setObjectName(QStringLiteral("actionQuit"));
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        cover_widget = new CoverGlWidget(centralWidget);
        cover_widget->setObjectName(QStringLiteral("cover_widget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(cover_widget->sizePolicy().hasHeightForWidth());
        cover_widget->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(cover_widget);

        cover_list = new QListWidget(centralWidget);
        cover_list->setObjectName(QStringLiteral("cover_list"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(cover_list->sizePolicy().hasHeightForWidth());
        cover_list->setSizePolicy(sizePolicy1);
        cover_list->setStyleSheet(QLatin1String("QListWidget::item {\n"
"	padding: 5px;\n"
"}\n"
""));
        cover_list->setAutoScrollMargin(20);

        horizontalLayout->addWidget(cover_list);

        MainWindowClass->setCentralWidget(centralWidget);

        retranslateUi(MainWindowClass);
        QObject::connect(actionQuit, SIGNAL(triggered()), MainWindowClass, SLOT(close()));

        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "MainWindow", 0));
        actionSave_Texture->setText(QApplication::translate("MainWindowClass", "Save Texture", 0));
        actionLoad_Texture->setText(QApplication::translate("MainWindowClass", "Load Texture", 0));
        actionQuit->setText(QApplication::translate("MainWindowClass", "Quit", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
