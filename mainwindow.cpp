#include <QtGui>
#include <QPrinter>

//#include "finddialog.h"
#include "mainwindow.h"


// ---------------------------------------------------------------------------
// PUBLIC CONSTRUCTOR
//

MainWindow::MainWindow()
{
    editor = new QTextEdit;
    editor->setLineWrapColumnOrWidth( QTextEdit::FixedColumnWidth );
    editor->setAcceptRichText( false );
    connect( newAction, SIGNAL( triggered() ), this, SLOT( newFile() ));
    setCentralWidget( editor );

    createActions();
    createMenus();
    createContextMenu();
    createStatusBar();

    readSettings();

//    findDialog = 0;

    connect( editor, SIGNAL( cursorPositionChanged() ), this, SLOT( updatePositionLabel() ));
    connect( editor->document(), SIGNAL( contentsChanged() ), this, SLOT( updateModified() ));

    setMinimumWidth( statusBar()->minimumWidth() + 20 );
//    setWindowIcon( QIcon(":/images/editor.png"));
    setWindowTitle( tr("Text Editor") );
    setCurrentFile("");

}


// ---------------------------------------------------------------------------
// OVERRIDDEN EVENTS
//

void MainWindow::closeEvent( QCloseEvent *event )
{
    if ( okToContinue() ) {
        writeSettings();
        event->accept();
    }
    else {
        event->ignore();
    }
}


// ---------------------------------------------------------------------------
// SLOTS
//

void MainWindow::newFile()
{
    if ( okToContinue() ) {
        editor->clear();
        setCurrentFile("");
    }
}


void MainWindow::open()
{
    if ( okToContinue() ) {
        QString fileName = QFileDialog::getOpenFileName( this,
                                                         tr("Open File"),
                                                         ".",
                                                         tr("All files (*)")
                                                       );
        if ( !fileName.isEmpty() )
            loadFile( fileName );
    }
}


bool MainWindow::save()
{
    if ( currentFile.isEmpty() )
        return saveAs();
    else
        return saveFile( currentFile );
}


bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName( this,
                                                     tr("Save File"), ".",
                                                     tr("All files (*)"));
    if ( fileName.isEmpty() )
        return false;
    return saveFile( fileName );
}


void MainWindow::find()
{
}


void MainWindow::replace()
{
}


void MainWindow::about()
{
    QMessageBox::about( this,
                        tr("Product Information"),
                        tr("<b>Quick Text Editor</b><br>Version %1<hr>"
                           "Copyright &copy;2018 Alexander Taylor"
                           "<p>Licensed under the GNU General Public License "
                           "version 3.0&nbsp;<br>"
                           "<a href=\"https://www.gnu.org/licenses/gpl.html\">"
                           "https://www.gnu.org/licenses/gpl.html</a>"
                           "<br></p>").arg( PROGRAM_VERSION )
                      );
}


void MainWindow::openRecentFile()
{
    if ( okToContinue() ) {
        QAction *action = qobject_cast<QAction *>( sender() );
        if ( action )
            loadFile( action->data().toString() );
    }
}


void MainWindow::clearRecentFiles()
{
    int r = QMessageBox::question( this,
                                   tr("Clear List?"),
                                   tr("Clear the list of recent files?"),
                                   QMessageBox::Yes | QMessageBox::No
                                 );
    if ( r == QMessageBox::Yes ) {
        recentFiles.clear();
        updateRecentFileActions();
    }
}


void MainWindow::toggleEditMode( bool ovr )
{
    editor->setOverwriteMode( ovr );
    updateModeLabel();
}


void MainWindow::toggleWordWrap()
{
    QTextOption::WrapMode mode = editor->wordWrapMode();
    if ( mode == QTextOption::WrapAtWordBoundaryOrAnywhere )
        mode = QTextOption::NoWrap;
    else
        mode = QTextOption::WrapAtWordBoundaryOrAnywhere;
    editor->setWordWrapMode( mode );
}


void MainWindow::toggleReadOnly( bool ovr )
{
    setReadOnly( ovr );
}


void MainWindow::updateStatusBar()
{
    updateModeLabel();
    updatePositionLabel();
}


void MainWindow::updateModeLabel()
{
    if ( editor->isReadOnly() )
        editModeLabel->setText(" RO ");
    else if ( editor->overwriteMode() )
        editModeLabel->setText(" OVR ");
    else
        editModeLabel->setText(" INS ");
}


void MainWindow::updatePositionLabel()
{
    QTextCursor cursor;
    int row = 0,
        col = 0;

    cursor = editor->textCursor();
    col = cursor.positionInBlock();
    row = cursor.blockNumber() + 1;
    positionLabel->setText( QString("%1:%2").arg( row ).arg( col ));
}


void MainWindow::updateModified()
{
    bool isModified = editor->document()->isModified();
    setWindowModified( isModified );
    modifiedLabel->setText( isModified? tr("Modified"): "");
    messagesLabel->setText("");
}
void MainWindow::updateModified( bool isModified )
{
    editor->document()->setModified( isModified );
    setWindowModified( isModified );
    modifiedLabel->setText( isModified? tr("Modified"): "");
    messagesLabel->setText("");
}


void MainWindow::setEditorFont() {
    bool fontSelected;
    QFont font = QFontDialog::getFont( &fontSelected, editor->font(), this );
    if ( fontSelected )
        editor->setFont( font );
}


// ---------------------------------------------------------------------------
// OTHER METHODS
//

void MainWindow::createActions()
{

    // File menu actions

    newAction = new QAction( tr("&New"), this );
    newAction->setShortcut( QKeySequence::New );
    newAction->setStatusTip( tr("Create a new file") );
    connect( newAction, SIGNAL( triggered() ), this, SLOT( newFile() ));

    openAction = new QAction( tr("&Open..."), this );
    openAction->setShortcut( QKeySequence::Open );
    openAction->setStatusTip( tr("Open a file") );
    connect( openAction, SIGNAL( triggered() ), this, SLOT( open() ));

    saveAction = new QAction( tr("&Save"), this );
    QList<QKeySequence> saveShortcuts;
    saveShortcuts << QKeySequence("F2") << QKeySequence("Ctrl+S");
    saveAction->setShortcuts( saveShortcuts );
    saveAction->setStatusTip( tr("Save the current file") );
    connect( saveAction, SIGNAL( triggered() ), this, SLOT( save() ));

    saveAsAction = new QAction( tr("Save &as..."), this );
    saveAsAction->setShortcut( QKeySequence::SaveAs );
    saveAsAction->setStatusTip( tr("Save the current file under a new name") );
    connect( saveAsAction, SIGNAL( triggered() ), this, SLOT( saveAs() ));

    printAction = new QAction( tr("&Print..."), this );
    printAction->setShortcut( QKeySequence::Print );
    printAction->setStatusTip( tr("Print the current file") );
    connect( printAction, SIGNAL( triggered() ), this, SLOT( print() ));

    for ( int i = 0; i < MaxRecentFiles; i++ )
    {
        recentFileActions[ i ] = new QAction( this );
        recentFileActions[ i ]->setVisible( false );
        connect( recentFileActions[ i ], SIGNAL( triggered() ), this, SLOT( openRecentFile() ));
    }

    clearRecentAction = new QAction( tr("&Clear list"), this );
    clearRecentAction->setVisible( false );
    clearRecentAction->setStatusTip( tr("Clear the list of recent files") );
    connect( clearRecentAction, SIGNAL( triggered() ), this, SLOT( clearRecentFiles() ));

    exitAction = new QAction( tr("E&xit"), this );
    exitAction->setShortcut( tr("F3") );
    exitAction->setStatusTip( tr("Exit the program") );
    connect( exitAction, SIGNAL( triggered() ), this, SLOT( close() ));


    // Edit menu actions

    undoAction = new QAction( tr("&Undo"), this );
    undoAction->setShortcut( QKeySequence::Undo );
    undoAction->setStatusTip( tr("Undo the previous edit") );
    connect( undoAction, SIGNAL( triggered() ), editor, SLOT( undo() ));

    redoAction = new QAction( tr("&Redo"), this );
    redoAction->setShortcut( QKeySequence::Redo );
    redoAction->setStatusTip( tr("Revert the previous undo operation") );
    connect( redoAction, SIGNAL( triggered() ), editor, SLOT( redo() ));

    cutAction = new QAction( tr("&Cut"), this );
    cutAction->setShortcut( QKeySequence::Cut );
    cutAction->setStatusTip( tr("Move the selected text to the clipboard") );
    connect( cutAction, SIGNAL( triggered() ), editor, SLOT( cut() ));

    copyAction = new QAction( tr("&Copy"), this );
    copyAction->setShortcut( QKeySequence::Copy );
    copyAction->setStatusTip( tr("Copy the selected text to the clipboard") );
    connect( copyAction, SIGNAL( triggered() ), editor, SLOT( copy() ));

    pasteAction = new QAction( tr("&Paste"), this );
    pasteAction->setShortcut( QKeySequence::Paste );
    pasteAction->setStatusTip( tr("Insert the current clipboard text") );
    connect( pasteAction, SIGNAL( triggered() ), editor, SLOT( paste() ));

    selectAllAction = new QAction( tr("Select &all"), this );
    selectAllAction->setShortcut( QKeySequence::SelectAll );
    selectAllAction->setStatusTip( tr("Select all text in the edit window") );
    connect( selectAllAction, SIGNAL( triggered() ), editor, SLOT( selectAll() ));


    // Options menu actions

    wrapAction = new QAction( tr("&Word wrap"), this );
    wrapAction->setCheckable( true );
    wrapAction->setChecked(
        (editor->wordWrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere)?
        true: false
    );
    wrapAction->setShortcut( tr("Alt+W") );
    wrapAction->setStatusTip( tr("Toggle word wrap") );
    connect( wrapAction, SIGNAL( toggled( bool )), this, SLOT( toggleWordWrap() ));

    editModeAction = new QAction( tr("&Overwrite mode"), this );
    editModeAction->setCheckable( true );
    editModeAction->setChecked( editor->overwriteMode() );
    editModeAction->setShortcut( tr("Ins") );
    editModeAction->setStatusTip( tr("Toggle overwrite mode") );
    connect( editModeAction, SIGNAL( toggled( bool )), this, SLOT( toggleEditMode( bool )));

    readOnlyAction = new QAction( tr("&Read-only mode"), this );
    readOnlyAction->setCheckable( true );
    readOnlyAction->setChecked( editor->isReadOnly() );
    readOnlyAction->setShortcut( tr("Alt+R") );
    readOnlyAction->setStatusTip( tr("Toggle read-only mode") );
    connect( readOnlyAction, SIGNAL( toggled( bool )), this, SLOT( toggleReadOnly( bool )));

    fontAction = new QAction( tr("&Font..."), this );
    fontAction->setShortcut( tr("Alt+F") );
    fontAction->setStatusTip( tr("Change the edit window font") );
    connect( fontAction, SIGNAL( triggered() ), this, SLOT( setEditorFont() ));


    aboutAction = new QAction( tr("&Product information"), this );
    aboutAction->setStatusTip( tr("Show product information") );
    connect( aboutAction, SIGNAL( triggered() ), this, SLOT( about() ));
}


void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu( tr("&File") );
    fileMenu->addAction( newAction );
    fileMenu->addAction( openAction );
    fileMenu->addAction( saveAction );
    fileMenu->addAction( saveAsAction );
    fileMenu->addSeparator();
    fileMenu->addAction( printAction );
    separatorAction = fileMenu->addSeparator();
    for ( int i = 0; i < MaxRecentFiles; i++ )
        fileMenu->addAction( recentFileActions[ i ] );
    fileMenu->addAction( clearRecentAction );
    fileMenu->addSeparator();
    fileMenu->addAction( exitAction );

    editMenu = menuBar()->addMenu( tr("&Edit") );
    editMenu->addAction( undoAction );
    editMenu->addAction( redoAction );
    editMenu->addSeparator();
    editMenu->addAction( cutAction );
    editMenu->addAction( copyAction );
    editMenu->addAction( pasteAction );
    editMenu->addSeparator();
    editMenu->addAction( selectAllAction );

    optionsMenu = menuBar()->addMenu( tr("&Options") );
    optionsMenu->addAction( fontAction );
    optionsMenu->addAction( wrapAction );
    optionsMenu->addAction( editModeAction );
    optionsMenu->addAction( readOnlyAction );

    menuBar()->addSeparator();
    helpMenu = menuBar()->addMenu( tr("&Help") );
    helpMenu->addAction( aboutAction );

}


void MainWindow::createContextMenu()
{
}


void MainWindow::createStatusBar()
{
    editModeLabel = new QLabel(" OVR ");
    editModeLabel->setAlignment( Qt::AlignHCenter );
    editModeLabel->setMinimumSize( editModeLabel->sizeHint() );

    positionLabel = new QLabel("000000:000000");
    positionLabel->setAlignment( Qt::AlignHCenter );
    positionLabel->setMinimumSize( positionLabel->sizeHint() );

    messagesLabel = new QLabel("                                       ");
    messagesLabel->setIndent( 3 );
    messagesLabel->setMinimumSize( messagesLabel->sizeHint() );

    modifiedLabel = new QLabel(" Modified ");
    modifiedLabel->setAlignment( Qt::AlignHCenter );
    modifiedLabel->setMinimumSize( modifiedLabel->sizeHint() );

    statusBar()->addWidget( messagesLabel, 1 );
    statusBar()->addWidget( editModeLabel );
    statusBar()->addWidget( positionLabel );
    statusBar()->addWidget( modifiedLabel );
    statusBar()->setMinimumSize( statusBar()->sizeHint() );

    messagesLabel->setForegroundRole( QPalette::ButtonText );
    editModeLabel->setForegroundRole( QPalette::ButtonText );
    positionLabel->setForegroundRole( QPalette::ButtonText );
    modifiedLabel->setForegroundRole( QPalette::ButtonText );

    updateStatusBar();
}


void MainWindow::readSettings()
{
    QSettings settings( SETTINGS_VENDOR, SETTINGS_APP );

    restoreGeometry( settings.value("geometry").toByteArray() );

    recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFileActions();

    toggleEditMode( settings.value("overwrite", false ).toBool() );

    bool wrap = settings.value("wrapMode", QTextOption::WrapAtWordBoundaryOrAnywhere ).toBool();
    editor->setWordWrapMode( wrap? QTextOption::WrapAtWordBoundaryOrAnywhere: QTextOption::NoWrap );

    QString defaultFont;
    QFontDatabase fontdb;
    QStringList matching = fontdb.families().filter("Monotype Sans Duospace WT");
    if ( !matching.isEmpty() )
        defaultFont = matching.at( 0 );
    else
        defaultFont = "Courier";
    QFont font("");
    font.fromString( settings.value("editorFont", defaultFont ).toString() );
    editor->setFont( font );
}


void MainWindow::writeSettings()
{
    QSettings settings( SETTINGS_VENDOR, SETTINGS_APP );

    settings.setValue("geometry",    saveGeometry() );
    settings.setValue("recentFiles", recentFiles );
    settings.setValue("overwrite",   editor->overwriteMode() );
    settings.setValue("wrapMode",
                      (editor->wordWrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere )?
                      true: false
                     );
    settings.setValue("editorFont",  editor->font().toString() );
}


bool MainWindow::okToContinue()
{
    if ( isWindowModified() ) {
        int r = QMessageBox::warning( this,
                                      tr("Text Editor"),
                                      tr("There are unsaved changes. "
                                         "Do you want to save the changes?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                                    );
        if ( r == QMessageBox::Save )
            return save();
        else if ( r == QMessageBox::Cancel )
            return false;
    }
    return true;
}


bool MainWindow::loadFile( const QString &fileName )
{
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly | QFile::Text )) {
        QMessageBox::critical( this, tr("Error"), tr("The file could not be opened."));
        return false;
    }
    QTextStream in( &file );
    QString text = in.readAll();
    editor->setText( text );
    file.close();

    setCurrentFile( fileName );
    QString displayName = fileName;
    showMessage( tr("Opened file: %1").arg( displayName.replace( QString("/"), QString("\\"))));
    return true;
}


bool MainWindow::saveFile( const QString &fileName )
{
    QFile file( fileName );
    if ( !file.open( QIODevice::WriteOnly | QFile::Text )) {
        QMessageBox::critical( this, tr("Error"), tr("Error writing file"));
        return false;
    }
    QTextStream out( &file );
    QString text = editor->toPlainText();
    out << text;
    file.flush();
    file.close();

    setCurrentFile( fileName );
    QString displayName = fileName;
    showMessage( tr("Saved file: %1").arg( displayName.replace( QString("/"), QString("\\"))));
    return true;
}


bool MainWindow::print()
{
    QPrinter printer( QPrinter::HighResolution );
    QPrintDialog printDialog( &printer, this );
    if ( printDialog.exec() ) {
        editor->print( &printer );
        return true;
    }
    else
        return false;
}


void MainWindow::setCurrentFile( const QString &fileName )
{
    currentFile = fileName;
    updateModified( false );
    QString shownName = tr("Untitled");
    if ( !currentFile.isEmpty() ) {
        shownName = strippedName( currentFile );
        recentFiles.removeAll( currentFile );
        recentFiles.prepend( currentFile );
        updateRecentFileActions();
    }
    setWindowTitle( tr("Text Editor - %1 [*]").arg( shownName ));
}


void MainWindow::updateRecentFileActions()
{
    QMutableStringListIterator i( recentFiles );
    while ( i.hasNext() ) {
        if ( !QFile::exists( i.next() )) {
            i.remove();
        }
    }
    for ( int j = 0; j < MaxRecentFiles; j++ ) {
        if ( j < recentFiles.count() ) {
            QString text = tr("&%1 %2").arg( j+1 ).arg( strippedName( recentFiles[ j ] ));
            recentFileActions[ j ]->setText( text );
            recentFileActions[ j ]->setData( recentFiles[ j ] );
            recentFileActions[ j ]->setVisible( true );
        }
        else {
            recentFileActions[ j ]->setVisible( false );
        }
    }
    separatorAction->setVisible( !recentFiles.isEmpty() );
    clearRecentAction->setVisible( !recentFiles.isEmpty() );
}


QString MainWindow::strippedName( const QString &fullFileName )
{
    return QFileInfo( fullFileName ).fileName();
}


void MainWindow::showMessage( const QString &message )
{
    messagesLabel->setText( message );
}


void MainWindow::showUsage()
{
    QMessageBox::information( this, tr("Usage"),
                              tr("qe [ <i>filename</i> [ /read ]] &nbsp; | &nbsp; [ /h | /? ]"),
                              QMessageBox::Ok
                            );
}


void MainWindow::setReadOnly( bool readOnly )
{
    editor->setReadOnly( readOnly );
    updateModeLabel();
}