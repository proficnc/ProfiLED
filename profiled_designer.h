#ifndef PROFILED_DESIGNER_H
#define PROFILED_DESIGNER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "design_scene.h"
#include <QDebug>
#include <QColor>
#include <QColorDialog>
#include <QTimer>

class QAction;
class QActionGroup;
class QLabel;
class QMenu;

namespace Ui {
class profiled_designer;
}

class profiled_designer : public QMainWindow
{
    Q_OBJECT

public:
    explicit profiled_designer(QWidget *parent = 0);
    ~profiled_designer();

private:
    design_scene *scene;
    Ui::profiled_designer *ui;
    void createActions();
    void createMenus();
    QMenu *fileMenu;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    qint8 selected_led_id;
    pattern curr_pattern;
    QTimer *timer;
    void update_params(bool update_list);
protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU
signals:

private slots:
    void add_pattern_handler(bool action);
    void led_selected_handler(qint8 led_id);
    void start_color_select_handler(bool action);
    void end_color_select_handler(bool action);
    void color_select_handler(bool action);
    void color_dialog_handler(QColor color);
    void play_button_handler(bool action);
    void pause_button_handler(bool action);
    void newFile();
    void open();
    void save();
};

#endif // PROFILED_DESIGNER_H
