#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	for (int i = 0; i < sizeof(coverName) / sizeof(coverName[0]); i++)
	{
		std::string backPath = coverPathBackSmall[i];
		std::string frontPath = coverPathFront[i];
		std::string name = coverName[i];

		QListWidgetItem *item = new QListWidgetItem(QIcon(backPath.c_str()), name.c_str(), ui.cover_list);
		ui.cover_list->addItem(item);
	}

	setWindowTitle("Online Shopping - iPhone 7 Plus Phone Case");
	setWindowIcon(QIcon("./Resources/Images/icon.png"));

	ui.cover_list->setIconSize(QSize(62, 118));
	ui.cover_list->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);

	connect(ui.cover_list, SIGNAL(currentRowChanged(int)), ui.cover_widget, SLOT(coverChanged(int)));
	connect(ui.cover_widget, SIGNAL(selectionModeChanged(bool)), this, SLOT(changeSelectionMode(bool)));
	connect(ui.cover_widget, SIGNAL(selectionButtonChanged(bool)), this, SLOT(changeSelectedItem(bool)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::changeSelectionMode(bool selecting)
{
	if (selecting)
	{
		int currentRow = ui.cover_list->currentRow();
		if (currentRow == -1) currentRow = 0;
		ui.cover_list->setCurrentRow(currentRow);
		ui.cover_list->setFocus();
		ui.cover_list->setStyleSheet("QListWidget::item {\n	padding: 5px;\n}\nQListWidget {\n	outline:none;\n	border: 2px solid #4f9dfc;\n border-radius: 4px;\n}\n");
		QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
		shadow->setBlurRadius(25);
		shadow->setOffset(0);
		shadow->setColor(QColor("#4f9dfc"));
		ui.cover_list->setGraphicsEffect(shadow);
	}
	else
	{
		ui.cover_list->setStyleSheet("QListWidget::item {\n	padding: 5px;\n}");
		ui.cover_list->clearFocus();
		ui.cover_list->setGraphicsEffect(0);
	}
}

void MainWindow::changeSelectedItem(bool upButtonPressed)
{
	int itemCount = ui.cover_list->count();
	int currentRow = ui.cover_list->currentRow();

	if (upButtonPressed)
	{
		if (currentRow == 0) currentRow = itemCount;
		ui.cover_list->setCurrentRow((currentRow - 1) % itemCount);
	}
	else
	{
		ui.cover_list->setCurrentRow((currentRow + 1) % itemCount);
	}
}