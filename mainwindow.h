#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_Inicio_clicked();
    void on_pushButton_AgregarCita_clicked();
    void on_pushButton_MostrarCita_clicked();
    void actualizarHabilitacionBotonEliminar();
    void actualizarHabilitacionBotonHistorial();
    void mostrarPacienteDeFila(int fila, int columna);
    void on_pushButton_eliminarCaducadas_clicked();
    void on_pushButton_BuscarCita_clicked();
    void on_pushButton_mostrarHistorial_clicked();
    void on_pushButton_eliminarCita_clicked();
    void on_pushButton_guardarHistorial_clicked();
    void manejarOrdenTabla(int index);
    void buscarCitasPorCoincidencia();
    void actualizarEspecialistas(const QString &especialidad);
    void guardarEnBD();
    void mostrarCitas_tableWidget_citas();
    void mostrarCitas_tablaCitas();
private:
    void conectaBaseDeDatos();
    Ui::MainWindow *ui;
    QRegularExpression m_regexDigitos;
    QRegularExpression m_regexEspeciales;
    QRegularExpression regexTelefono;
    QRegularExpression regexCedula;
    QRegularExpression regexCorreo;

    QMap<QString, QStringList> especialistas_por_especialidad;

    enum opcionesOrden {
        opcionNeutra = 0,
        opcionAlfabetico = 1,
        opcionFechaReciente = 2,
        opcionFechaAntigua = 3
    };
};

#endif // MAINWINDOW_H
