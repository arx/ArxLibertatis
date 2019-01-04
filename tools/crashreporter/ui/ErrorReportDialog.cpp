/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "crashreporter/ui/ErrorReportDialog.h"

#include "ui_ErrorReportDialog.h"

#include "platform/Platform.h"

ScreenshotWidget::ScreenshotWidget(QWidget * parent) : QWidget(parent) { }

bool ScreenshotWidget::load(const QString & fileName) {
	return m_pixmap.load(fileName);
}

void ScreenshotWidget::setPixmap(const QPixmap & pixmap) {
	m_pixmap = pixmap;
}

void ScreenshotWidget::paintEvent(QPaintEvent * event) {
	
	ARX_UNUSED(event);
	
	QPainter p(this);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	
	QPixmap scaledPixmap = m_pixmap.scaled(size(), Qt::KeepAspectRatio);
	p.drawPixmap(0, 0, scaledPixmap);
	
}

ErrorReportDialog::ErrorReportDialog(ErrorReport & errorReport, QWidget * parent)
	: QDialog(parent)
	, ui(new Ui::ErrorReportDialog)
	, m_pCurrentTask(0)
	, m_errorReport(errorReport)
{
	
	ui->setupUi(this);
	
	ui->stackedWidget->setCurrentIndex(0);
	ui->lblProgressTitle->setText("");
	ui->progressBar->setMaximum(0);
	ui->progressBar->setValue(0);
	
	ErrorReportFileListModel * model = new ErrorReportFileListModel(errorReport, ui->listFiles);
	QItemSelectionModel * selectionModel = new QItemSelectionModel(model);
	
	ui->listFiles->setModel(model);
	ui->listFiles->setSelectionModel(selectionModel);
	ui->listFiles->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(ui->listFiles->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(onShowFileContent(const QItemSelection &, const QItemSelection &)));
	
	m_fileViewHex.setAsciiArea(false);
	m_fileViewHex.setReadOnly(true);
	m_fileViewHex.setAddressArea(false);
	
	ui->pageImage->layout()->addWidget(&m_fileViewImage);
	ui->pageBinary->layout()->addWidget(&m_fileViewHex);
	
	startTask(new GatherInfoTask(m_errorReport), Pane_Welcome);
	
	setWindowIcon(QApplication::windowIcon());
}

ErrorReportDialog::~ErrorReportDialog() {
	delete ui;
}

void ErrorReportDialog::onBack() {
	ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() - 1);
}

void ErrorReportDialog::onNext() {
	ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void ErrorReportDialog::onSend() {
	
	if(!ui->lineEditUsername->text().isEmpty()) {
		m_errorReport.SetLoginInfo(ui->lineEditUsername->text(), ui->lineEditPassword->text());
	}
	m_errorReport.SetReproSteps(ui->textEditReproSteps->toPlainText());
	
	startTask(new SendReportTask(m_errorReport), Pane_ExitSuccess);
	
}

void ErrorReportDialog::onPaneChanged(int index) {
	
	switch(index) {
		
		case Pane_Progress: {
			ui->btnBack->setEnabled(false);
			ui->btnNext->setEnabled(false);
			ui->btnSend->setEnabled(false);
			break;
		}
		
		case Pane_Welcome: {
			ui->btnBack->setEnabled(false);
			ui->btnNext->setEnabled(true);
			ui->btnSend->setEnabled(false);
			break;
		}
		
		case Pane_CrashDetails: {
			ui->btnBack->setEnabled(true);
			ui->btnNext->setEnabled(true);
			ui->btnSend->setEnabled(false);
			ui->textEditErrorDescription->setText(m_errorReport.GetErrorDescription());
			break;
		}
		
		case Pane_AttachedFiles: {
			ui->btnBack->setEnabled(true);
			ui->btnNext->setEnabled(true);
			ui->btnSend->setEnabled(false);
			QModelIndex idx = ui->listFiles->model()->index(0, 0);
			if(idx.isValid()) {
				ui->listFiles->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
			}
			break;
		}
		
		case Pane_ReproSteps: {
			ui->btnBack->setEnabled(true);
			ui->btnNext->setEnabled(true);
			ui->btnSend->setEnabled(false);
			break;
		}
		
		case Pane_Send: {
			ui->btnBack->setEnabled(true);
			ui->btnNext->setEnabled(false);
			ui->btnSend->setEnabled(true);
			break;
		}
		
		case Pane_ExitSuccess:
		case Pane_ExitError: {
			ui->btnBack->setEnabled(false);
			ui->btnNext->setEnabled(false);
			ui->btnSend->setEnabled(false);
			ui->btnClose->setEnabled(true);
			break;
		}
		
		default: arx_unreachable();
		
	}
	
}

void ErrorReportDialog::onShowFileContent(const QItemSelection & newSelection,
                                          const QItemSelection & oldSelection) {
	
	ARX_UNUSED(oldSelection);
	const QModelIndexList selectedIndexes = newSelection.indexes();
	if(selectedIndexes.empty()) {
		return;
	}
	
	const QModelIndex selectedIndex = selectedIndexes.at(0);
	if(!selectedIndex.isValid()) {
		return;
	}
	
	QString fileName = m_errorReport.GetAttachedFiles()[selectedIndex.row()].path;
	QString ext = QFileInfo(fileName).suffix().toLower();
	if(ext == "txt" || ext == "log" || ext == "ini" || ext == "xml") {
		QFile textFile(fileName);
		textFile.open(QIODevice::ReadOnly | QIODevice::Text);
		ui->fileViewText->setText(QString::fromUtf8(textFile.readAll()));
		ui->stackedFileViews->setCurrentIndex(0);
	} else if(ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "png" || ext == "gif") {
		m_fileViewImage.load(fileName);
		ui->stackedFileViews->setCurrentIndex(1);
	} else {
		QFile binaryFile(fileName);
		binaryFile.open(QIODevice::ReadOnly);
		if(binaryFile.size() > 20 * 1024 * 1024) {
			ui->stackedFileViews->setCurrentIndex(3);
			return;
		}
		m_fileViewHex.setData(binaryFile.readAll());
		ui->stackedFileViews->setCurrentIndex(2);
	}
	
}

void ErrorReportDialog::onTaskStarted(const QString & taskDescription, int numSteps) {
	ui->lblProgressTitle->setText(taskDescription);
	ui->progressBar->setMaximum(numSteps);
	ui->progressBar->setValue(0);
}

void ErrorReportDialog::onTaskStepStarted(const QString & taskStepDescription) {
	QString textDescription = QString("(%1/%2) %3...").arg(ui->progressBar->value() + 1)
	                                                  .arg(ui->progressBar->maximum())
	                                                  .arg(taskStepDescription);
	ui->lblProgressDescription->setText(textDescription);
}

void ErrorReportDialog::onTaskStepEnded() {
	ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void ErrorReportDialog::onTaskCompleted() {
	
	if(m_pCurrentTask->getErrorString().isEmpty()) {
		
		QString link = m_errorReport.GetIssueLink();
		QString prefix = "https://bugs.arx-libertatis.org/arx/issues/";
		if(link.startsWith(prefix)) {
			link = "http://arx.vg/" + link.mid(prefix.length());
		}
		QString htmlLink = QString("<a href=\"%1\">%1</a>").arg(link);
		ui->lblIssueLink->setText(htmlLink);
		
		const QList<QString> & files = m_errorReport.getFailedFiles();
		if(!files.empty()) {
			QString message = "<b>The following files could not be uploaded:</b><br><br>";
			Q_FOREACH(const QString & file, files) {
				message += file + "<br>";
			}
			message += "<br>Please attach them manually!";
			ui->lblFailedFiles->setText(message);
			ui->lblFailedFiles->setTextInteractionFlags(Qt::TextSelectableByMouse);
		}
		
		ui->stackedWidget->setCurrentIndex(m_nextPane);
		
	} else {
		ui->lblExitFail->setText(m_pCurrentTask->getErrorString());
		ui->lblExitFailDetails->setText(m_pCurrentTask->getDetailedErrorString());
		ui->stackedWidget->setCurrentIndex(Pane_ExitError);
	}
	
	m_pCurrentTask->deleteLater();
	m_pCurrentTask = 0;
	
}

void ErrorReportDialog::startTask(CrashReportTask * pTask, int nextPane) {
	
	if(m_pCurrentTask) {
		m_pCurrentTask->deleteLater();
	}
	
	m_pCurrentTask = pTask;
	
	connect(m_pCurrentTask, SIGNAL(taskStarted(const QString &, int)), SLOT(onTaskStarted(const QString &, int)));
	connect(m_pCurrentTask, SIGNAL(taskStepStarted(const QString &)), SLOT(onTaskStepStarted(const QString &)));
	connect(m_pCurrentTask, SIGNAL(taskStepEnded()), SLOT(onTaskStepEnded()));
	connect(m_pCurrentTask, SIGNAL(finished()), SLOT(onTaskCompleted()));
	
	ui->stackedWidget->setCurrentIndex(Pane_Progress);
	m_nextPane = nextPane;
	
	m_pCurrentTask->start();
}

GatherInfoTask::GatherInfoTask(ErrorReport & errorReport) : CrashReportTask(errorReport, 0) { }

void GatherInfoTask::run() {
	m_errorReport.GenerateReport(this);
}

SendReportTask::SendReportTask(ErrorReport & errorReport) : CrashReportTask(errorReport, 0) { }

void SendReportTask::run() {
	// Send mail
	m_errorReport.SendReport(this);
}
