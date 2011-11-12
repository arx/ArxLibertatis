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

#ifndef ARX_CRASHREPORTER_ERRORREPORTDIALOG_H
#define ARX_CRASHREPORTER_ERRORREPORTDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QThread>
#include <QSemaphore>

#include "errorreport.h"
#include "qhexedit/qhexedit.h"
#include "xmlhighlighter/xmlhighlighter.h"


namespace Ui {
	class ErrorReportDialog;
}

class CrashReportTask : public QThread
{
	Q_OBJECT

public:
	CrashReportTask(ErrorReport& errorReport, const QString& strDescription, int numSteps, QObject* parent = 0)
		: QThread(parent)
		, m_strDescription(strDescription)
		, m_numSteps(numSteps)
		, m_errorReport(errorReport)
	{
	}

	const QString& getDescription() const { return m_strDescription; }
	int getNumSteps() const { return m_numSteps; }

	bool succeeded() const { return isFinished() && m_strErrorDescription.isEmpty(); }
	const QString& getErrorString() const { return m_strErrorDescription; }

signals:
	void taskStepStarted(const QString& taskStepDescription);
	void taskStepEnded();

protected:
	void setError(const QString& strError)
	{
		if(m_strErrorDescription.isEmpty() && !strError.isEmpty())
			m_strErrorDescription = strError;
	}

protected:
	ErrorReport& m_errorReport;

private:
	const QString m_strDescription;
	const int m_numSteps;
	QString m_strErrorDescription;
};

class GatherInfoTask : public CrashReportTask
{
	Q_OBJECT

public:
	GatherInfoTask(ErrorReport& errorReport);

private:
	void run();
};

class SendReportTask : public CrashReportTask
{
	Q_OBJECT

public:
	SendReportTask(ErrorReport& errorReport);

private:
	void run();
};

class ScreenshotWidget : public QWidget {
	Q_OBJECT

public:
	ScreenshotWidget(QWidget *parent = 0);

	bool load(const QString& fileName);
	void setPixmap(const QPixmap& pixmap);

protected:
	void paintEvent(QPaintEvent *event);

private:
	QPixmap m_pixmap;
};

class ErrorReportDialog : public QDialog
{
	Q_OBJECT

public:
	enum DialogPane
	{
		Pane_Progress,
		Pane_FillInfo,
		Pane_ExitSuccess,
		Pane_ExitError
	};

public:
	explicit ErrorReportDialog(ErrorReport& errorReport, QWidget *parent = 0);
	~ErrorReportDialog();

	void SetCurrentPane(DialogPane paneId);

	// Progress pane
	void ResetProgressBar(int maxNumber);
	void IncrementProgress();
	void SetProgressText(const QString& strProgress);

	// Exit pane
	void SetExitText(const QString& strExit);

public slots:
	void onTaskStepStarted(const QString& taskStepDescription);
	void onTaskStepEnded();
	void onTaskCompleted();

private slots:
	void onSendReport();
	void onShowFileContent();

private:
	void startTask(CrashReportTask* pTask, int nextPane);

	Ui::ErrorReportDialog *ui;
	QHexEdit m_fileViewHex;
	ScreenshotWidget m_fileViewImage;
	XmlHighlighter* m_pXmlHighlighter;

	CrashReportTask* m_pCurrentTask;
	int m_nextPane;

	ErrorReport& m_errorReport;
};

#endif // ARX_CRASHREPORTER_ERRORREPORTDIALOG_H
