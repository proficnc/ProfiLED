#include <QtWidgets>
#include "profiled_designer.h"
#include "ui_profiled_designer.h"

profiled_designer::profiled_designer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::profiled_designer),
    selected_led_id(-1)
{
    ui->setupUi(this);
    //Set Scene for LED Design scene
    scene = new design_scene(this);
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::black,Qt::SolidPattern));
    createActions();
    createMenus();
    // Add the vertical lines first, paint them red
    for (int x=0; x<=1000; x+=20) {
        scene->addLine(x,0,x,1000, QPen(Qt::white));
    }
    // Now add the horizontal lines, paint them green
    for (int y=0; y<=1000; y+=20) {
        scene->addLine(0,y,1000,y, QPen(Qt::white));
    }
    //Setup 10ms Timer
    timer = new QTimer(this);

    //Connect Signals and slots
    QObject::connect(scene,SIGNAL(led_selected(qint8)), this, SLOT(led_selected_handler(qint8)));
    QObject::connect(ui->color_select, SIGNAL(clicked(bool)), this, SLOT(color_select_handler(bool)));
    QObject::connect(ui->start_color_select, SIGNAL(clicked(bool)), this, SLOT(start_color_select_handler(bool)));
    QObject::connect(ui->end_color_select, SIGNAL(clicked(bool)), this, SLOT(end_color_select_handler(bool)));
    QObject::connect(ui->add_pattern, SIGNAL(clicked(bool)), this, SLOT(add_pattern_handler(bool)));
    QObject::connect(ui->play_button, SIGNAL(clicked(bool)), this, SLOT(play_button_handler(bool)));
    QObject::connect(ui->pause_button, SIGNAL(clicked(bool)), this, SLOT(pause_button_handler(bool)));
    QObject::connect(ui->create_bin, SIGNAL(clicked(bool)), this, SLOT(create_bin_handler(bool)));
    QObject::connect(ui->remove_pattern, SIGNAL(clicked(bool)), this, SLOT(remove_pattern_handler(bool)));
    QObject::connect(timer, SIGNAL(timeout()), scene->get_led_strip(), SLOT(loop_player()));
}

void profiled_designer::play_button_handler(bool action)
{
    scene->set_loop_time(ui->loop_duration->value());
    timer->start(10);
}

void profiled_designer::pause_button_handler(bool action)
{
    timer->stop();
}

void profiled_designer::create_bin_handler(bool action)
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("LED Designer Binary Files"), tr(".ledbin"));
    scene->save_patterns_to_file(fileName);
}

void profiled_designer::remove_pattern_handler(bool action)
{

}

void profiled_designer::led_selected_handler(qint8 led_id)
{
    selected_led_id = led_id;
    ui->led_id->setText(QString::number(led_id));
    //update and populate pattern list
    update_params(true);
}

void profiled_designer::update_params(bool update_list)
{
    qint16 consumed_time = 0;
    qint16 global_total_time = ui->loop_duration->value();
    QList<pattern> pattern_list = scene->get_pattern_list(selected_led_id);
    //empty pattern list
    if(update_list) {
        ui->pattern_list->clear();
    }
    for(int i = 0; i < pattern_list.length(); i++) {
        if(update_list) {
            ui->pattern_list->addItem(pattern_list[i].toString());
        }
        consumed_time += pattern_list[i].offset + pattern_list[i].total_time;
    }
    //reset colors and values
    ui->total_time->setMaximum(global_total_time*100 - consumed_time);
    if(global_total_time*100 - consumed_time <= 0) {
        ui->offset_time->setMaximum(0);
    } else {
        ui->offset_time->setMaximum(global_total_time*100 - consumed_time - 1);
    }
}

void profiled_designer::color_select_handler(bool action)
{
    if(selected_led_id == -1 || !ui->solid_color->isChecked()) {
        return;
    }

    QColorDialog *color_dialog = new QColorDialog(Qt::white,this);
    QObject::connect(color_dialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(color_dialog_handler(QColor)));
    QObject::connect(color_dialog, SIGNAL(colorSelected(QColor)), this, SLOT(color_dialog_handler(QColor)));
    color_dialog->show();
}

void profiled_designer::color_dialog_handler(QColor color)
{
    scene->set_led_color(selected_led_id, color);
    QPalette palette;
    QColor neg_color(255 - color.red(), 255 - color.green(), 255 - color.blue());
    palette.setColor(QPalette::Base,color);
    palette.setColor(QPalette::Text,neg_color);
    ui->solid_color_text->setPalette(palette);
    ui->solid_color_text->setText(color.name().toUpper());
    curr_pattern.start_color = color;
    curr_pattern.end_color = color;
}

void profiled_designer::start_color_select_handler(bool action)
{
    if(selected_led_id == -1 || !ui->pattern->isChecked()) {
        return;
    }
    curr_pattern.start_color = QColorDialog::getColor(Qt::white, this);
    scene->set_led_color(selected_led_id, curr_pattern.start_color);
    QPalette palette;
    QColor neg_color(255 - curr_pattern.start_color.red(),
                     255 - curr_pattern.start_color.green(),
                     255 - curr_pattern.start_color.blue());
    palette.setColor(QPalette::Base,curr_pattern.start_color);
    palette.setColor(QPalette::Text,neg_color);
    ui->start_color->setPalette(palette);
    ui->start_color->setText(curr_pattern.start_color.name().toUpper());
}

void profiled_designer::end_color_select_handler(bool action)
{
    if(selected_led_id == -1 || !ui->pattern->isChecked()) {
        return;
    }
    curr_pattern.end_color = QColorDialog::getColor(Qt::white, this);
    QPalette palette;
    QColor neg_color(255 - curr_pattern.end_color.red(),
                     255 - curr_pattern.end_color.green(),
                     255 - curr_pattern.end_color.blue());
    palette.setColor(QPalette::Base,curr_pattern.end_color);
    palette.setColor(QPalette::Text,neg_color);
    ui->end_color->setPalette(palette);
    ui->end_color->setText(curr_pattern.end_color.name().toUpper());
}

void profiled_designer::add_pattern_handler(bool action)
{
    if(selected_led_id == -1) {
        return;
    }
    qint16 global_total_time = ui->loop_duration->value();

    if(ui->pattern->isChecked()) {
        curr_pattern.is_solid = false;
    } else {
        curr_pattern.is_solid = true;
    }
    curr_pattern.offset = ui->offset_time->value();
    curr_pattern.mid = ui->pattern_mid->value();
    curr_pattern.total_time = ui->total_time->value();
    scene->push_led_pattern(selected_led_id ,curr_pattern);
    ui->pattern_list->addItem(curr_pattern.toString());
    //update and populate pattern list
    update_params(false);
}

#ifndef QT_NO_CONTEXTMENU
void profiled_designer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU

void profiled_designer::newFile()
{
}

void profiled_designer::open()
{
}

void profiled_designer::save()
{
}

void profiled_designer::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &profiled_designer::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &profiled_designer::open);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &profiled_designer::save);
}

void profiled_designer::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
}

profiled_designer::~profiled_designer()
{
    delete ui;
}
