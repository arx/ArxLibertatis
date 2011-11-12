/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "errorreportdialog.h"
#include "ui_errorreportdialog.h"


ScreenshotWidget::ScreenshotWidget(QWidget *parent) : QWidget(parent)
{
}

bool ScreenshotWidget::load(const QString& fileName)
{
	return m_pixmap.load(fileName);
}

void ScreenshotWidget::setPixmap(const QPixmap& pixmap)
{
	m_pixmap = pixmap;
}

void ScreenshotWidget::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	p.drawPixmap(rect(), m_pixmap, m_pixmap.rect());
}


ErrorReportDialog::ErrorReportDialog(ErrorReport& errorReport, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ErrorReportDialog),
	m_pCurrentTask(0),
	m_errorReport(errorReport)
{
	ui->setupUi(this);

	ui->sendReportButtonBox->buttons().at(0)->setText("Send");
	connect(ui->sendReportButtonBox, SIGNAL(accepted()), SLOT(onSendReport()));

	ui->stackedWidget->setCurrentIndex(0);
	ui->lblProgressTitle->setText("");
	ui->progressBar->setMaximum(0);
	ui->progressBar->setValue(0);

	QListWidgetItem* pItem1 = new QListWidgetItem("screenshot.jpg");
	QListWidgetItem* pItem2 = new QListWidgetItem("crash.dmp");
	QListWidgetItem* pItem3 = new QListWidgetItem("crash.xml");
	ui->listFiles->addItem(pItem1);
	ui->listFiles->addItem(pItem2);
	ui->listFiles->addItem(pItem3);
	connect(ui->listFiles, SIGNAL(itemSelectionChanged()), SLOT(onShowFileContent()));
	ui->listFiles->selectedItems().insert(0, pItem1);

	m_fileViewHex.setAsciiArea(false);
	m_fileViewHex.setReadOnly(true);
	m_fileViewHex.setAddressArea(false);

	m_pXmlHighlighter = new XmlHighlighter(ui->fileViewXml);

	ui->pageImage->layout()->addWidget(&m_fileViewImage);
	ui->pageBinary->layout()->addWidget(&m_fileViewHex);

	startTask(new GatherInfoTask(m_errorReport), Pane_FillInfo);
}

ErrorReportDialog::~ErrorReportDialog()
{
	delete m_pXmlHighlighter;
	delete ui;
}

void ErrorReportDialog::onSendReport()
{
	startTask(new SendReportTask(m_errorReport), Pane_ExitSuccess);
}

void ErrorReportDialog::onShowFileContent()
{
	QString fileName = ui->listFiles->selectedItems().at(0)->text();
	if(fileName.endsWith(".txt"))
	{
		QFile textFile(fileName);
		textFile.open(QIODevice::ReadOnly | QIODevice::Text);

		QByteArray data;
		data = textFile.readAll();

		QString textFileContent(data);

		ui->fileViewText->setText(textFileContent);
		ui->stackedFileViews->setCurrentIndex(0);
	}
	else if(fileName.endsWith(".xml"))
	{
		QFile textFile(fileName);
		textFile.open(QIODevice::ReadOnly | QIODevice::Text);

		QByteArray data;
		data = textFile.readAll();

		QString textFileContent(data);

		ui->fileViewXml->setText(textFileContent);
		ui->stackedFileViews->setCurrentIndex(1);
	}
	else if(fileName.endsWith(".jpg"))
	{
		m_fileViewImage.load(fileName);
		ui->stackedFileViews->setCurrentIndex(2);
	}
	else
	{
		QFile binaryFile(fileName);
		binaryFile.open(QIODevice::ReadOnly);

		QByteArray data;
		data = binaryFile.readAll();

		m_fileViewHex.setData(data);
		ui->stackedFileViews->setCurrentIndex(3);
	}
}

void ErrorReportDialog::onTaskStepStarted(const QString& taskStepDescription)
{
	QString textDescription = QString("(%1/%2) %3...")
								.arg(ui->progressBar->value()+1)
								.arg(ui->progressBar->maximum())
								.arg(taskStepDescription);

	ui->lblProgressDescription->setText(textDescription);
}

void ErrorReportDialog::onTaskStepEnded()
{
	ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void ErrorReportDialog::onTaskCompleted()
{
	if(m_pCurrentTask->succeeded())
	{
		ui->stackedWidget->setCurrentIndex(m_nextPane);
	}
	else
	{
		ui->lblExitFail->setText(m_pCurrentTask->getErrorString());
		ui->stackedWidget->setCurrentIndex(Pane_ExitError);
	}

	delete m_pCurrentTask;
	m_pCurrentTask = 0;
}

void ErrorReportDialog::startTask(CrashReportTask* pTask, int nextPane)
{
	delete m_pCurrentTask;
	m_pCurrentTask = pTask;

	ui->lblProgressTitle->setText(pTask->getDescription());
	ui->progressBar->setMaximum(pTask->getNumSteps());
	ui->progressBar->setValue(0);

	connect(m_pCurrentTask, SIGNAL(taskStepStarted(const QString&)), SLOT(onTaskStepStarted(const QString&)));
	connect(m_pCurrentTask, SIGNAL(taskStepEnded()), SLOT(onTaskStepEnded()));
	connect(m_pCurrentTask, SIGNAL(finished()), SLOT(onTaskCompleted()));

	ui->stackedWidget->setCurrentIndex(Pane_Progress);
	m_nextPane = nextPane;

	m_pCurrentTask->start();
}

GatherInfoTask::GatherInfoTask(ErrorReport& errorReport) : CrashReportTask(errorReport, "Generating crash report", 5, 0)
{
}

void GatherInfoTask::run()
{
	m_errorReport.Initialize();

	// Take screenshot
	taskStepStarted("Grabbing screenshot");
	bool bScreenshot = m_errorReport.GetScreenshot("screenshot.jpg");
	taskStepEnded();

	// Generate minidump
	taskStepStarted("Generating minidump");
	bool bCrashDump = m_errorReport.GetCrashDump("crash.dmp");
	taskStepEnded();

	// Gather machine info
	//taskStepStarted("Gathering system information");
	//bool bMachineInfo = m_errorReport.GetMachineInfo("machineInfo.txt");
	//taskStepEnded();

	// Generate xml
	taskStepStarted("Generating report manifest");
	bool bCrashXml = m_errorReport.WriteReport("crash.xml");
	taskStepEnded();

	// Generate archive
	taskStepStarted("Compressing report");
	bool bCrashArchive = m_errorReport.GenerateArchive();
	taskStepEnded();

	m_errorReport.ReleaseApplicationLock();
/*
	taskStepStarted("Screenshot");
	sleep(1);
	taskStepEnded();

	taskStepStarted("CrashDmp");
	sleep(1);
	taskStepEnded();

	taskStepStarted("MachineInfo");
	sleep(1);
	taskStepEnded();

	taskStepStarted("Xml");
	sleep(1);
	taskStepEnded();

	taskStepStarted("Zip");
	sleep(1);
	taskStepEnded();
*/
}

SendReportTask::SendReportTask(ErrorReport& errorReport) : CrashReportTask(errorReport, "Sending crash report", 3, 0)
{
}

void SendReportTask::run()
{
	// Send mail
	bool bSentMail = m_errorReport.Send();
	/*
	taskStepStarted("Connect");
	sleep(1);
	taskStepEnded();

	taskStepStarted("Authenticate");
	sleep(1);
	taskStepEnded();

	taskStepStarted("Send");
	sleep(1);
	taskStepEnded();
	*/
}
