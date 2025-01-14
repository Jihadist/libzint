/*
    Zint Barcode Generator - the open source barcode generator
    Copyright (C) 2009 - 2021 Robin Stuart <rstuart114@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/* vim: set ts=4 sw=4 et : */

//#include <QDebug>
#include <QUiLoader>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStringBuilder>

#include "exportwindow.h"

// Shorthand
#define QSL QStringLiteral

ExportWindow::ExportWindow(BarcodeItem *bc, const QString& output_data) : m_bc(bc), m_output_data(output_data)
{
    setupUi(this);
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif

    QByteArray geometry = settings.value(QSL("studio/export/window_geometry")).toByteArray();
    restoreGeometry(geometry);

    linDestPath->setText(settings.value(QSL("studio/export/destination"),
                        QDir::toNativeSeparators(QDir::homePath())).toString());
    linPrefix->setText(settings.value(QSL("studio/export/file_prefix"), QSL("bcs_")).toString());
    cmbFileName->setCurrentIndex(settings.value(QSL("studio/export/name_format"), 0).toInt());
    cmbFileFormat->setCurrentIndex(settings.value(QSL("studio/export/filetype"), 0).toInt());

    QIcon closeIcon(QIcon::fromTheme(QSL("window-close"), QIcon(QSL(":res/x.svg"))));
    btnCancel->setIcon(closeIcon);

    connect(btnCancel, SIGNAL( clicked( bool )), SLOT(close()));
    connect(btnOK, SIGNAL( clicked( bool )), SLOT(process()));
    connect(btnDestPath, SIGNAL( clicked( bool )), SLOT(get_directory()));
}

ExportWindow::~ExportWindow()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    settings.setValue(QSL("studio/export/window_geometry"), saveGeometry());

    settings.setValue(QSL("studio/export/destination"), linDestPath->text());
    settings.setValue(QSL("studio/export/file_prefix"), linPrefix->text());
    settings.setValue(QSL("studio/export/name_format"), cmbFileName->currentIndex());
    settings.setValue(QSL("studio/export/filetype"), cmbFileFormat->currentIndex());
}

void ExportWindow::get_directory()
{
    QSettings settings;
#if QT_VERSION < 0x60000
    settings.setIniCodec("UTF-8");
#endif
    QString directory;
    QFileDialog fdialog;

    fdialog.setFileMode(QFileDialog::Directory);
    fdialog.setDirectory(settings.value(QSL("studio/default_dir"),
                QDir::toNativeSeparators(QDir::homePath())).toString());

    if (fdialog.exec()) {
        directory = fdialog.selectedFiles().at(0);
    } else {
        return;
    }

    linDestPath->setText(QDir::toNativeSeparators(directory));
    settings.setValue(QSL("studio/default_dir"), directory);
}

void ExportWindow::process()
{
    const QRegularExpression urlRE(QSL("[\\/:*?\"<>|%]"));

    txtFeedback->setPlainText(tr("Processing..."));
    txtFeedback->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QStringList dataList = m_output_data.split('\n');
    int lines = dataList.size();
    if (lines && dataList[lines - 1].isEmpty()) {
        lines--;
    }

    QString biggest;
    bool needUrlEscape = false;

    const int fileNameIdx = cmbFileName->currentIndex();
    if (fileNameIdx == 1) {
        biggest = QString::number(lines + 1);
    } else {
        needUrlEscape = m_output_data.contains(urlRE);
    }

    QString suffix;
    switch (cmbFileFormat->currentIndex()) {
#ifdef NO_PNG
        case 0: suffix = QSL(".eps"); break;
        case 1: suffix = QSL(".gif"); break;
        case 2: suffix = QSL(".svg"); break;
        case 3: suffix = QSL(".bmp"); break;
        case 4: suffix = QSL(".pcx"); break;
        case 5: suffix = QSL(".emf"); break;
        case 6: suffix = QSL(".tif"); break;
#else
        case 0: suffix = QSL(".png"); break;
        case 1: suffix = QSL(".eps"); break;
        case 2: suffix = QSL(".gif"); break;
        case 3: suffix = QSL(".svg"); break;
        case 4: suffix = QSL(".bmp"); break;
        case 5: suffix = QSL(".pcx"); break;
        case 6: suffix = QSL(".emf"); break;
        case 7: suffix = QSL(".tif"); break;
#endif
    }

    QString filePathPrefix = linDestPath->text() % QDir::separator() % linPrefix->text();

    QStringList Feedback;
    int successCount = 0, errorCount = 0;
    for (int i = 0; i < lines; i++) {
        const QString &dataString = dataList[i];
        QString fileName;
        switch (fileNameIdx) {
            case 0: /* Same as Data (URL Escaped) */
                if (needUrlEscape) {
                    QString url_escaped;

                    for (int m = 0; m < dataString.length(); m++) {
                        QChar name_qchar = dataString[m];
                        char name_char = name_qchar.toLatin1();

                        switch (name_char) {
                            case '\\': url_escaped += QSL("%5C"); break;
                            case '/': url_escaped += QSL("%2F"); break;
                            case ':': url_escaped += QSL("%3A"); break;
                            case '*': url_escaped += QSL("%2A"); break;
                            case '?': url_escaped += QSL("%3F"); break;
                            case '"': url_escaped += QSL("%22"); break;
                            case '<': url_escaped += QSL("%3C"); break;
                            case '>': url_escaped += QSL("%3E"); break;
                            case '|': url_escaped += QSL("%7C"); break;
                            case '%': url_escaped += QSL("%25"); break;
                            default: url_escaped += name_qchar; break;
                        }
                    }
                    fileName = filePathPrefix % url_escaped % suffix;
                } else {
                    fileName = filePathPrefix % dataString % suffix;
                }
                break;
            case 1: { /* Formatted Serial Number */
                    QString this_val, pad;

                    this_val = QString::number(i + 1);

                    pad.fill('0', biggest.length() - this_val.length());

                    fileName = filePathPrefix % pad % this_val % suffix;
                }
                break;
        }
        m_bc->bc.setText(dataString);
        m_bc->bc.save_to_file(fileName);
        if (m_bc->bc.hasErrors()) {
            /*: %1 is line number, %2 is error message */
            Feedback << tr("Line %1: %2").arg(i + 1).arg(m_bc->bc.error_message());
            errorCount++;
        } else {
            /*: %1 is line number */
            Feedback << tr("Line %1: Success").arg(i + 1);
            successCount++;
        }
        if (i && (i % 100 == 0)) {
            txtFeedback->appendPlainText(Feedback.join('\n'));
            txtFeedback->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            Feedback.clear();
        }
    }

    QString summary;
    if (errorCount && successCount) {
        /*: %1 is total no. of items processed, %2 is no. of failures, %3 is no. of successes */
        summary = tr("Total %1, %2 failed, %3 succeeded.").arg(errorCount + successCount).arg(errorCount)
                    .arg(successCount);
    } else if (errorCount) {
        /*: %1 is no. of failures */
        summary = tr("All %1 failed.").arg(errorCount);
    } else if (successCount) {
        /*: %1 is no. of successes */
        summary = tr("All %1 succeeded.").arg(successCount);
    } else {
        summary = tr("No items.");
    }
    if (Feedback.size()) {
        txtFeedback->appendPlainText(Feedback.join('\n') + '\n' + summary + '\n');
    } else {
        txtFeedback->appendPlainText(summary + '\n');
    }
    txtFeedback->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}
