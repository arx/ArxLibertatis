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

#ifndef ARX_TOOLS_CRASHREPORTER_UI_ERRORREPORTDIALOG_H
#define ARX_TOOLS_CRASHREPORTER_UI_ERRORREPORTDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QAbstractButton>
#include <QThread>
#include <QSemaphore>

#include "crashreporter/ErrorReport.h"
#include "crashreporter/qhexedit/QHexEdit.h"

namespace Ui {
	class ErrorReportDialog;
}

/*!
 * Base task for tasks
 */
class CrashReportTask : public QThread, public ErrorReport::IProgressNotifier {
	
	Q_OBJECT
	
public:
	
	CrashReportTask(ErrorReport & errorReport, QObject * parent = 0)
		: QThread(parent)
		, m_errorReport(errorReport)
	{ }
	
	/*!
	 * Get the error string (available in case of a failure)
	 * \return A string detailling the error that occured in case of a failure.
	 * @sa succeeded()
	 */
	const QString & getErrorString() const { return m_strErrorDescription; }
	const QString & getDetailedErrorString() const { return m_strDetailedErrorDescription; }
	
	static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
	
signals:
	
	void taskStarted(const QString & taskDescription, int numSteps);
	void taskStepStarted(const QString & taskStepDescription);
	void taskStepEnded();
	
protected:
	
	void setError(const QString & strError) {
		if(m_strErrorDescription.isEmpty() && !strError.isEmpty()) {
			m_strErrorDescription = strError;
		}
	}
	
	void setDetailedError(const QString & strDetailedError) {
		if(m_strDetailedErrorDescription.isEmpty() && !strDetailedError.isEmpty()) {
			m_strDetailedErrorDescription = strDetailedError;
		}
	}
	
protected:
	
	ErrorReport & m_errorReport;
	
private:
	
	QString m_strErrorDescription;
	QString m_strDetailedErrorDescription;
	
};

class GatherInfoTask : public CrashReportTask {
	
	Q_OBJECT
	
public:
	
	explicit GatherInfoTask(ErrorReport & errorReport);
	
private:
	
	void run();
	
};

class SendReportTask : public CrashReportTask {
	
	Q_OBJECT
	
public:
	
	explicit SendReportTask(ErrorReport & errorReport);
	
private:
	
	void run();
	
};

class ScreenshotWidget : public QWidget {
	
	Q_OBJECT
	
public:
	
	explicit ScreenshotWidget(QWidget * parent = 0);
	
	bool load(const QString & fileName);
	void setPixmap(const QPixmap & pixmap);
	
protected:
	
	void paintEvent(QPaintEvent * event);
	
private:
	
	QPixmap m_pixmap;
	
};

class ErrorReportFileListModel : public QAbstractListModel {
	
	Q_OBJECT
	
public:
	
	ErrorReportFileListModel(ErrorReport & errorReport, QObject * parent = 0)
		: QAbstractListModel(parent)
		, m_errorReport(errorReport)
	{ }
	
	int rowCount(const QModelIndex & /* parent */ = QModelIndex()) const {
		return int(m_errorReport.GetAttachedFiles().size());
	}
	
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const {
		
		if(index.row() < 0 || index.row() >= rowCount() || !index.isValid()) {
			return QVariant();
		}
		
		if(role == Qt::DisplayRole) {
			return QFileInfo(m_errorReport.GetAttachedFiles()[index.row()].path).fileName();
		} else if(role == Qt::CheckStateRole) {
			return m_errorReport.GetAttachedFiles()[index.row()].attachToReport ? Qt::Checked : Qt::Unchecked;
		}
		
		return QVariant();
	}
	
	bool setData(const QModelIndex & index, const QVariant & value, int role) {
		
		if(index.row() < 0 || index.row() >= rowCount() || !index.isValid()) {
			return false;
		}
		
		if(role == Qt::CheckStateRole) {
			m_errorReport.GetAttachedFiles()[index.row()].attachToReport
				= static_cast<Qt::CheckState>(value.toUInt()) == Qt::Checked;
		}
		
		return true;
	}
	
	QVariant headerData(int section, Qt::Orientation orientation, int role) const {
		if(role != Qt::DisplayRole) {
			return QVariant();
		} else if(orientation == Qt::Horizontal) {
			return QString("Column %1").arg(section);
		} else {
			return QString("Row %1").arg(section);
		}
	}
	
	Qt::ItemFlags flags(const QModelIndex & index) const {
		if(index.row() < 0 || index.row() >= rowCount() || !index.isValid()) {
			return Qt::NoItemFlags;
		}
		return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
private:
	
	ErrorReport & m_errorReport;
	
};

class ErrorReportDialog : public QDialog {
	
	Q_OBJECT
	
public:
	
	enum DialogPane {
		Pane_Progress,
		Pane_Welcome,
		Pane_CrashDetails,
		Pane_AttachedFiles,
		Pane_ReproSteps,
		Pane_Send,
		Pane_ExitSuccess,
		Pane_ExitError
	};
	
public:
	
	explicit ErrorReportDialog(ErrorReport & errorReport, QWidget * parent = 0);
	~ErrorReportDialog();
	
	void SetCurrentPane(DialogPane paneId);
	
	// Progress pane
	void ResetProgressBar(int maxNumber);
	void IncrementProgress();
	void SetProgressText(const QString & strProgress);
	
	// Exit pane
	void SetExitText(const QString & strExit);
	
private slots:
	
	void onTaskStarted(const QString & taskDescription, int numSteps);
	void onTaskStepStarted(const QString & taskStepDescription);
	void onTaskStepEnded();
	void onTaskCompleted();
	void onPaneChanged(int index);
	
	void onBack();
	void onNext();
	void onSend();
	void onShowFileContent(const QItemSelection & newSelection, const QItemSelection & oldSelection);
	
private:
	
	void startTask(CrashReportTask * pTask, int nextPane);
	
	Ui::ErrorReportDialog * ui;
	QHexEdit m_fileViewHex;
	ScreenshotWidget m_fileViewImage;
	
	CrashReportTask * m_pCurrentTask;
	int m_nextPane;
	
	ErrorReport & m_errorReport;
	
};

#endif // ARX_TOOLS_CRASHREPORTER_UI_ERRORREPORTDIALOG_H
