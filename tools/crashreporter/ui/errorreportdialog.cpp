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

#include "platform/Platform.h"

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

void ScreenshotWidget::paintEvent(QPaintEvent * event)
{
	ARX_UNUSED(event);
	QPainter p(this);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	p.drawPixmap(rect(), m_pixmap, m_pixmap.rect());
}

// This is needed to send signals with std strings
Q_DECLARE_METATYPE(std::string)

ErrorReportDialog::ErrorReportDialog(ErrorReport& errorReport, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ErrorReportDialog),
	m_pCurrentTask(0),
	m_errorReport(errorReport)
{
	// This is needed to send signals with std strings
	qRegisterMetaType<std::string>("std::string");
	
	ui->setupUi(this);

	ui->sendReportButtonBox->buttons().at(0)->setText("Send");

	ui->stackedWidget->setCurrentIndex(0);
	ui->lblProgressTitle->setText("");
	ui->progressBar->setMaximum(0);
	ui->progressBar->setValue(0);

	ErrorReportFileListModel* model = new ErrorReportFileListModel(errorReport, ui->listFiles);
	QItemSelectionModel* selectionModel = new QItemSelectionModel(model);

	ui->listFiles->setModel(model);
	ui->listFiles->setSelectionModel(selectionModel);
	ui->listFiles->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(ui->listFiles->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
            this, SLOT(onShowFileContent(const QItemSelection &, const QItemSelection &)));

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

void ErrorReportDialog::onTabChanged(int index)
{
	if(index == 1) // tabReportContent
	{
		QModelIndex idx = ui->listFiles->model()->index(0, 0);
		if(idx.isValid())
			ui->listFiles->selectionModel()->select(idx, QItemSelectionModel::Select);
	}
}

void ErrorReportDialog::onShowFileContent(const QItemSelection& newSelection, const QItemSelection & oldSelection)
{
	ARX_UNUSED(oldSelection);
	const QModelIndexList selectedIndexes = newSelection.indexes();
	if(selectedIndexes.empty())
		return;

	const QModelIndex selectedIndex = selectedIndexes.at(0);
	if(!selectedIndex.isValid())
		return;
	
	fs::path fileName = m_errorReport.GetAttachedFiles()[selectedIndex.row()];
	std::string ext = fileName.ext();
	if(ext == ".txt" || ext == ".log" || ext == ".ini")
	{
		QFile textFile(fileName.string().c_str());
		textFile.open(QIODevice::ReadOnly | QIODevice::Text);

		QByteArray data;
		data = textFile.readAll();

		QString textFileContent(data);

		ui->fileViewText->setText(textFileContent);
		ui->stackedFileViews->setCurrentIndex(0);
	}
	else if(ext == ".xml")
	{
		QFile textFile(fileName.string().c_str());
		textFile.open(QIODevice::ReadOnly | QIODevice::Text);

		QByteArray data;
		data = textFile.readAll();

		QString textFileContent(data);

		ui->fileViewXml->setText(textFileContent);
		ui->stackedFileViews->setCurrentIndex(1);
	}
	else if(ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".png" || ext == ".gif")
	{
		m_fileViewImage.load(fileName.string().c_str());
		ui->stackedFileViews->setCurrentIndex(2);
	}
	else
	{
		QFile binaryFile(fileName.string().c_str());
		binaryFile.open(QIODevice::ReadOnly);

		QByteArray data;
		data = binaryFile.readAll();

		m_fileViewHex.setData(data);
		ui->stackedFileViews->setCurrentIndex(3);
	}
}

void ErrorReportDialog::onTaskStarted(const std::string& taskDescription, int numSteps)
{
	ui->lblProgressTitle->setText(taskDescription.c_str());
	ui->progressBar->setMaximum(numSteps);
	ui->progressBar->setValue(0);
}

void ErrorReportDialog::onTaskStepStarted(const std::string& taskStepDescription)
{
	QString textDescription = QString("(%1/%2) %3...")
								.arg(ui->progressBar->value()+1)
								.arg(ui->progressBar->maximum())
								.arg(taskStepDescription.c_str());

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

	connect(m_pCurrentTask, SIGNAL(taskStarted(const std::string&, int)), SLOT(onTaskStarted(const std::string&, int)));
	connect(m_pCurrentTask, SIGNAL(taskStepStarted(const std::string&)), SLOT(onTaskStepStarted(const std::string&)));
	connect(m_pCurrentTask, SIGNAL(taskStepEnded()), SLOT(onTaskStepEnded()));
	connect(m_pCurrentTask, SIGNAL(finished()), SLOT(onTaskCompleted()));

	ui->stackedWidget->setCurrentIndex(Pane_Progress);
	m_nextPane = nextPane;

	m_pCurrentTask->start();
}

GatherInfoTask::GatherInfoTask(ErrorReport& errorReport) : CrashReportTask(errorReport, 0)
{
}

void GatherInfoTask::run()
{
	m_errorReport.GenerateReport(this);
}

SendReportTask::SendReportTask(ErrorReport& errorReport) : CrashReportTask(errorReport, 0)
{
}

void SendReportTask::run()
{
	// Send mail
	m_errorReport.SendReport(this);
}
