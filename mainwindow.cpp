#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "persona.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QInputDialog>
#include <QDate>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_regexDigitos("\\d"),
    m_regexEspeciales("[^a-zA-Z0-9 ]"),
    regexTelefono("^\\d{11}$"),
    regexCedula("^\\d{9}$"),
    regexCorreo("^[A-Za-z0-9._%+-]+@gmail\\.com$")

{
    ui->setupUi(this);
    this->setWindowTitle("Clinica NJ");
    this->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    conectaBaseDeDatos();
    ui->stackedWidget->setCurrentWidget(ui->page);


    especialistas_por_especialidad["Traumatologia"] = {"Dr. Damian Sotelo", "Dra. Ximena Briceño"};
    especialistas_por_especialidad["Pediatria"] = {"Dra. Valeria Tovar", "Dr. Matias Giraldo"};
    especialistas_por_especialidad["Cardiologia"] = {"Dr. Orlando Cifuentes", "Dra. Yamileth Rangel"};
    especialistas_por_especialidad["Odontologia"] = {"Dr. Sebastian Osorio", "Dra. Maura Quezada"};

    ui->tableWidget_citas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_citas->setSelectionMode(QAbstractItemView::SingleSelection);

    QHeaderView*header=ui->tableWidget_citas->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    QHeaderView*header2=ui->tablaCitas->horizontalHeader();
    header2->setSectionResizeMode(QHeaderView::ResizeToContents);
    QHeaderView*header3=ui->tableWidget->horizontalHeader();
    header3->setSectionResizeMode(QHeaderView::ResizeToContents);


    connect(ui->tableWidget_citas, &QTableWidget::cellClicked,
            this, &MainWindow::mostrarPacienteDeFila);
    connect(ui->tableWidget_citas, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::actualizarHabilitacionBotonEliminar);
    connect(ui->tableWidget_citas, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::actualizarHabilitacionBotonHistorial);
    connect(ui->comboBoxOrdenar, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::manejarOrdenTabla);

    connect(ui->lineEdit_nombrePaciente, &QLineEdit::textChanged, this, [this]() {
        ui->lineEdit_nombrePaciente->setStyleSheet("border: 1px solid #707070;");
    });
    connect(ui->lineEdit_cedula, &QLineEdit::textChanged, this, [this]() {
        ui->lineEdit_cedula->setStyleSheet("border: 1px solid #707070;");
    });
    connect(ui->lineEdit_telefono, &QLineEdit::textChanged, this, [this]() {
        ui->lineEdit_telefono->setStyleSheet("border: 1px solid #707070;");
    });
    connect(ui->lineEdit_direccion, &QLineEdit::textChanged, this, [this]() {
        ui->lineEdit_direccion->setStyleSheet("border: 1px solid #707070;");
    });
    connect(ui->lineEdit_correo, &QLineEdit::textChanged, this, [this]() {
        ui->lineEdit_correo->setStyleSheet("border: 1px solid #707070;");
    });
    connect(ui->comboBox_sexo, &QComboBox::currentIndexChanged, this, [this]() {
        ui->comboBox_sexo->setStyleSheet("border: 1px solid #707070;");
    });
    connect(ui->comboBox_especialidad, &QComboBox::currentIndexChanged, this, [this]() {
        ui->comboBox_especialidad->setStyleSheet("border: 1px solid #707070;");
    });
    connect(ui->comboBox_especialista, &QComboBox::currentIndexChanged, this, [this]() {
        ui->comboBox_especialista->setStyleSheet("border: 1px solid #707070;");
    });

    connect(ui->ingresarNombrePaciente, &QLineEdit::textChanged,
            this, &MainWindow::buscarCitasPorCoincidencia);
    connect(ui->matchFecha, &QDateEdit::dateChanged,
            this, &MainWindow::buscarCitasPorCoincidencia);
    connect(ui->ingresarNombreEspecialista, &QLineEdit::textChanged,
            this, &MainWindow::buscarCitasPorCoincidencia);
    connect(ui->comboBox_especialidad, &QComboBox::currentTextChanged,
            this, &MainWindow::actualizarEspecialistas);
    connect(ui->pushButton_guardarCita, &QPushButton::clicked,
            this, &MainWindow::guardarEnBD);


    ui->lineEdit_nombrePaciente->setPlaceholderText("Ingresar...");
    ui->lineEdit_cedula->setPlaceholderText("Ingresar...");
    ui->lineEdit_telefono->setPlaceholderText("Ingresar...");
    ui->lineEdit_direccion->setPlaceholderText("Ingresar...");
    ui->lineEdit_correo->setPlaceholderText("Ingresar...");
    ui->textEdit_motivo->setPlaceholderText("Ingresar...");

    ui->dateEdit_fechaNacimiento->setMaximumDate(QDate::currentDate());
    ui->dateEdit_fechaConsulta->setMinimumDate(QDate::currentDate());

    QRegularExpressionValidator *validatorTlf = new QRegularExpressionValidator(regexTelefono, this);
    ui->lineEdit_telefono->setValidator(validatorTlf);
    QRegularExpressionValidator *validatorCedula = new QRegularExpressionValidator(regexCedula, this);
    ui->lineEdit_cedula->setValidator(validatorCedula);
    QRegularExpressionValidator *validatorCorreo = new QRegularExpressionValidator(regexCorreo, this);
    ui->lineEdit_correo->setValidator(validatorCorreo);

    ui->pushButton_eliminarCita->setEnabled(false);
    ui->pushButton_guardarHistorial->setEnabled(false);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::conectaBaseDeDatos() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("clinica_nj.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Error al abrir la base de datos", db.lastError().text());
        return;
    }
    qDebug() << "Ruta de la base de datos:" << QDir::currentPath();

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS pacientes ("
            "cedula TEXT PRIMARY KEY,"
            "nombre TEXT,"
            "fecha_nac TEXT,"
            "sexo TEXT,"
            "numero_telefono TEXT,"
            "direccion TEXT,"
            "correo TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS citas ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "especialidad TEXT,"
            "especialista TEXT,"
            "fecha DATE,"
            "hora_inicio TEXT,"
            "hora_fin TEXT,"
            "motivo TEXT,"
            "cedula_paciente TEXT,"
            "FOREIGN KEY(cedula_paciente) REFERENCES pacientes(cedula))");

    query.exec("CREATE TABLE IF NOT EXISTS historial ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nombre TEXT,"
            "cedula_paciente TEXT,"
            "especialidad TEXT,"
            "especialista TEXT,"
            "fecha DATE,"
            "hora_inicio TEXT,"
            "hora_fin TEXT,"
            "motivo TEXT,"
            "observacion TEXT)");
}

void MainWindow::on_pushButton_Inicio_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->page);
}

void MainWindow::on_pushButton_AgregarCita_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

void MainWindow::on_pushButton_MostrarCita_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->page_3);
    manejarOrdenTabla(ui->comboBoxOrdenar->currentIndex());
}

void MainWindow::actualizarHabilitacionBotonEliminar() {
    bool fila_seleccionada = ui->tableWidget_citas->currentRow() != -1;
    ui->pushButton_eliminarCita->setEnabled(fila_seleccionada);
}

void MainWindow::actualizarHabilitacionBotonHistorial() {
    bool fila_seleccionada = ui->tableWidget_citas->currentRow() != -1;
    ui->pushButton_guardarHistorial->setEnabled(fila_seleccionada);
}

void MainWindow::on_pushButton_guardarHistorial_clicked() {
    int fila_actual = ui->tableWidget_citas->currentRow();
    if (fila_actual < 0) {
        QMessageBox::warning(this, "Sin seleccion", "Selecciona una fila para eliminar");
        return;
    }

    QString nombre = ui->tableWidget_citas->item(fila_actual, 0)->text();
    QString cedula = ui->tableWidget_citas->item(fila_actual, 1)->text();
    QString especialidad = ui->tableWidget_citas->item(fila_actual, 2)->text();
    QString especialista = ui->tableWidget_citas->item(fila_actual, 3)->text();
    QString fechaConsulta = ui->tableWidget_citas->item(fila_actual, 4)->text();
    QString hora_inicio = ui->tableWidget_citas->item(fila_actual, 5)->text();
    QString hora_fin = ui->tableWidget_citas->item(fila_actual, 6)->text();
    QString motivo = ui->tableWidget_citas->item(fila_actual, 7)->text();

    bool observacion_ok;
    QString observacion = QInputDialog::getText(this, "Observaciones", "Ingrese observaciones (opcional):",
                            QLineEdit::Normal, "", &observacion_ok);
    if (!observacion_ok) {
        return;
    }
    if (observacion.isEmpty()) {
        observacion = "Sin observaciones";
    }

    QSqlDatabase::database().transaction();

    QSqlQuery queryHistorial;

    bool cambio_historial = true;

    queryHistorial.prepare("INSERT INTO historial (nombre, cedula_paciente, especialidad, especialista, fecha, hora_inicio, hora_fin, motivo, observacion) "
                           "VALUES (:nombre, :cedula, :especialidad, :especialista, :fecha, :hora_inicio, :hora_fin, :motivo, :observacion)");
    queryHistorial.bindValue(":nombre", nombre);
    queryHistorial.bindValue(":cedula", cedula);
    queryHistorial.bindValue(":especialidad", especialidad);
    queryHistorial.bindValue(":especialista", especialista);
    queryHistorial.bindValue(":fecha", fechaConsulta);
    queryHistorial.bindValue(":hora_inicio", hora_inicio);
    queryHistorial.bindValue(":hora_fin", hora_fin);
    queryHistorial.bindValue(":motivo", motivo);
    queryHistorial.bindValue(":observacion", observacion);

    cambio_historial &= queryHistorial.exec();

    queryHistorial.prepare("DELETE FROM citas WHERE cedula_paciente = :cedula");
    queryHistorial.bindValue(":cedula", cedula);

    cambio_historial &= queryHistorial.exec();

    queryHistorial.prepare("DELETE FROM pacientes WHERE cedula = :cedula");
    queryHistorial.bindValue(":cedula", cedula);

    cambio_historial &= queryHistorial.exec();

    if (cambio_historial){
        QSqlDatabase::database().commit();
        QMessageBox::information(this, "Cambio Exitoso", "La cita se ha guardado en el historial.");
    }
    else {
        QSqlDatabase::database().rollback();
        QMessageBox::critical(this, "Error", "Se produjo un fallo en el cambio.");
    }

    mostrarCitas_tableWidget_citas();
}


void MainWindow::on_pushButton_BuscarCita_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->page_4);
    mostrarCitas_tablaCitas();
}

void MainWindow::on_pushButton_mostrarHistorial_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->page_5);

    QSqlQuery query;
    query.prepare("SELECT nombre, cedula_paciente, especialidad, especialista, fecha, hora_inicio, hora_fin, motivo, observacion FROM historial");
    if (!query.exec()) {
        qDebug() << "Error al imprimir el historial: " << query.lastError().text();
    }

    ui->tableWidget->setRowCount(0);
    int fila = 0;
    while (query.next()) {
        ui->tableWidget->insertRow(fila);

        ui->tableWidget->setItem(fila, 0, new QTableWidgetItem(query.value(0).toString())); // nombre
        ui->tableWidget->setItem(fila, 1, new QTableWidgetItem(query.value(1).toString())); // cedula
        ui->tableWidget->setItem(fila, 2, new QTableWidgetItem(query.value(2).toString())); // especialidad
        ui->tableWidget->setItem(fila, 3, new QTableWidgetItem(query.value(3).toString())); // especialista
        ui->tableWidget->setItem(fila, 4, new QTableWidgetItem(query.value(4).toString())); // fecha
        ui->tableWidget->setItem(fila, 5, new QTableWidgetItem(query.value(5).toString())); // hora de inicio
        ui->tableWidget->setItem(fila, 6, new QTableWidgetItem(query.value(6).toString())); // hora de cierre
        ui->tableWidget->setItem(fila, 7, new QTableWidgetItem(query.value(7).toString())); // motivo
        ui->tableWidget->setItem(fila, 8, new QTableWidgetItem(query.value(8).toString())); // observacion

        fila++;
    }
}

void MainWindow::on_pushButton_eliminarCita_clicked() {
    int filaSeleccionada = ui->tableWidget_citas->currentRow();

    if (filaSeleccionada < 0) {
        QMessageBox::warning(this, "Sin selección", "Selecciona una cita para eliminar.");
        return;
    }

    QString id_cita = ui->tableWidget_citas->item(filaSeleccionada, 0)->text();
    QString cedula_paciente = ui->tableWidget_citas->item(filaSeleccionada, 1)->text();

    // Confirmación
    QMessageBox::StandardButton confirmacion = QMessageBox::question(
        this, "Eliminar cita",
        "¿Estás seguro de que deseas eliminar esta cita?",
        QMessageBox::Yes | QMessageBox::No);

    if (confirmacion == QMessageBox::Yes) {
        QSqlDatabase db = QSqlDatabase::database();
        db.transaction();

        QSqlQuery query(db);

        query.prepare("DELETE FROM citas WHERE id = :id");
        query.bindValue(":id", id_cita);
        if (!query.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Error", "No se pudo eliminar la cita:\n" + query.lastError().text());
            return;
        }

        query.prepare("DELETE FROM pacientes WHERE cedula = :cedula");
        query.bindValue(":cedula", cedula_paciente);
        if (!query.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Error", "No se pudo eliminar el paciente: " + query.lastError().text());
            return;
        }
        db.commit();

        ui->tableWidget_citas->removeRow(filaSeleccionada);
        QMessageBox::information(this, "Cita eliminada", "La cita ha sido eliminada correctamente.");
    }
}

void MainWindow::on_pushButton_eliminarCaducadas_clicked() {
    QMessageBox::StandardButton confirmacion = QMessageBox::question(
        this, "Eliminar citas caducadas",
        "¿Estás seguro de realizar esta acción?",
        QMessageBox::Yes | QMessageBox::No);

    if (confirmacion == QMessageBox::Yes) {
        QSqlDatabase::database().transaction();

        QSqlQuery queryPaciente;
        QSqlQuery queryCitas;
        bool eliminar = true;

        queryCitas.prepare("DELETE FROM citas WHERE fecha < :fecha");
        queryCitas.bindValue(":fecha", QDate::currentDate().toString("yyyy-MM-dd"));
        eliminar &= queryCitas.exec();

        queryPaciente.prepare("DELETE FROM pacientes "
                              "WHERE cedula NOT IN (SELECT cedula_paciente FROM citas)");
        eliminar &= queryPaciente.exec();

        if (eliminar) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, "Citas eliminadas", "Las citas caducadas han sido eliminadas exitosamente.");
        }
        else {
            QSqlDatabase::database().rollback();
            QMessageBox::critical(this, "Error", "Se ha producido un error al eliminar.");
        }
        mostrarCitas_tableWidget_citas();

    }
}

void MainWindow::manejarOrdenTabla(int index) {
    if (ui->stackedWidget->currentWidget() != ui->page_3) {
        return;
    }

    QString ordenTablaSQL;
    switch(index) {
        case opcionNeutra:
            ordenTablaSQL = "";
            break;
        case opcionAlfabetico:
            ordenTablaSQL = "ORDER BY p.nombre ASC";
            break;
        case opcionFechaReciente:
            ordenTablaSQL = "ORDER BY c.fecha DESC";
            break;

        case opcionFechaAntigua:
            ordenTablaSQL = "ORDER BY c.fecha ASC";
            break;
    }

    QString consulta_tabla = "SELECT p.nombre, p.cedula, c.especialidad, c.especialista, c.fecha, c.hora_inicio, c.hora_fin, c.motivo "
                    "FROM pacientes p JOIN citas c ON p.cedula = c.cedula_paciente " + ordenTablaSQL;

    QSqlQuery query;
    if (!query.exec(consulta_tabla)) {
        qDebug() << "Error: " << query.lastError().text();
    }

    ui->tableWidget_citas->setRowCount(0);
    int fila = 0;
    while (query.next()) {
        ui->tableWidget_citas->insertRow(fila);

        ui->tableWidget_citas->setItem(fila, 0, new QTableWidgetItem(query.value(0).toString())); // nombre
        ui->tableWidget_citas->setItem(fila, 1, new QTableWidgetItem(query.value(1).toString())); // cedula
        ui->tableWidget_citas->setItem(fila, 2, new QTableWidgetItem(query.value(2).toString())); // especialidad
        ui->tableWidget_citas->setItem(fila, 3, new QTableWidgetItem(query.value(3).toString())); // especialista
        ui->tableWidget_citas->setItem(fila, 4, new QTableWidgetItem(query.value(4).toString())); // fecha
        ui->tableWidget_citas->setItem(fila, 5, new QTableWidgetItem(query.value(5).toString())); // hora de inicio
        ui->tableWidget_citas->setItem(fila, 6, new QTableWidgetItem(query.value(6).toString())); // hora de cierre
        ui->tableWidget_citas->setItem(fila, 7, new QTableWidgetItem(query.value(7).toString())); // motivo
        fila++;
    }
}

void MainWindow::buscarCitasPorCoincidencia() {
    QString nombre_paciente = ui->ingresarNombrePaciente->text();
    QString nombre_especialista= ui->ingresarNombreEspecialista->text();
    QDate fecha_busqueda = ui->matchFecha->date();
    QString consulta = "SELECT p.nombre, p.cedula, c.especialidad, c.especialista, c.fecha, c.hora_inicio, c.hora_fin, c.motivo "
                       "FROM pacientes p JOIN citas c ON p.cedula = c.cedula_paciente";

    QList<QString> condiciones;
    QMap<QString, QVariant> valores;

    if (!nombre_paciente.isEmpty()) {
        condiciones.append("p.nombre LIKE :nombre");
        valores[":nombre"] = "%" + nombre_paciente + "%";
    }

    if (!nombre_especialista.isEmpty()) {
        condiciones.append("c.especialista LIKE :especialista");
        valores[":especialista"] = "%" + nombre_especialista + "%";
    }

    QDate fecha_predet(2000, 1, 1);
    if (fecha_busqueda != fecha_predet) {
        condiciones.append("c.fecha = :fecha");
        valores[":fecha"] = fecha_busqueda.toString("yyyy-MM-dd");
    }

    if (!condiciones.isEmpty()) {
        consulta += " WHERE " + condiciones.join(" AND ");
    }

    QSqlQuery queryBuscar;
    queryBuscar.prepare(consulta);

    // Asignar los valores a los placeholders
    for(auto it = valores.constBegin(); it != valores.constEnd(); ++it) {
        queryBuscar.bindValue(it.key(), it.value());
    }

    if (!queryBuscar.exec()) {
        qDebug() << "Error en la búsqueda combinada: " << queryBuscar.lastError().text();
        return;
    }
    // Finalmente, refrescar la tabla con los resultados
    ui->tablaCitas->setRowCount(0);
    int fila = 0;
    while (queryBuscar.next()) {
        ui->tablaCitas->insertRow(fila);

        ui->tablaCitas->setItem(fila, 0, new QTableWidgetItem(queryBuscar.value(0).toString()));
        ui->tablaCitas->setItem(fila, 1, new QTableWidgetItem(queryBuscar.value(1).toString()));
        ui->tablaCitas->setItem(fila, 2, new QTableWidgetItem(queryBuscar.value(2).toString()));
        ui->tablaCitas->setItem(fila, 3, new QTableWidgetItem(queryBuscar.value(3).toString()));
        ui->tablaCitas->setItem(fila, 4, new QTableWidgetItem(queryBuscar.value(4).toString()));
        ui->tablaCitas->setItem(fila, 5, new QTableWidgetItem(queryBuscar.value(5).toString()));
        ui->tablaCitas->setItem(fila, 6, new QTableWidgetItem(queryBuscar.value(6).toString()));
        ui->tablaCitas->setItem(fila, 7, new QTableWidgetItem(queryBuscar.value(7).toString()));
        fila++;
    }

}

    void MainWindow::actualizarEspecialistas(const QString &especialidad) {
        ui->comboBox_especialista->clear();
        ui->comboBox_especialista->addItem("Especialista:");
        ui->comboBox_especialista->addItems(especialistas_por_especialidad[especialidad]);
    }

    void MainWindow::guardarEnBD() {
        QMessageBox::StandardButton confirmacion = QMessageBox::question(
            this, "Registrar cita",
            "¿Desea continuar con el proceso? Se recomienda revisar los datos ingresados.",
            QMessageBox::Yes | QMessageBox::No);

        if (confirmacion == QMessageBox::Yes) {
        ui->lineEdit_nombrePaciente->setStyleSheet("border: 1px solid #707070;");
        ui->lineEdit_cedula->setStyleSheet("border: 1px solid #707070;");
        ui->comboBox_sexo->setStyleSheet("border: 1px solid #707070;");
        ui->lineEdit_telefono->setStyleSheet("border: 1px solid #707070;");
        ui->lineEdit_direccion->setStyleSheet("border: 1px solid #707070;");
        ui->lineEdit_correo->setStyleSheet("border: 1px solid #707070;");

        ui->comboBox_especialidad->setStyleSheet("border: 1px solid #707070;");
        ui->comboBox_especialista->setStyleSheet("border: 1px solid #707070;");

        //Datos del Paciente
        QString nombre = ui->lineEdit_nombrePaciente->text();
        QString cedula = ui->lineEdit_cedula->text();
        QString fechaNacimiento = ui->dateEdit_fechaNacimiento->date().toString("yyyy-MM-dd");
        QString sexo = ui->comboBox_sexo->currentText();
        QString telefono = ui->lineEdit_telefono->text();
        QString direccion = ui->lineEdit_direccion->text();
        QString correo = ui->lineEdit_correo->text();
        //Indice del comboBox para el sexo
        int indice_sexo = ui->comboBox_sexo->currentIndex();
        //Datos de la Cita
        QString especialidad = ui->comboBox_especialidad->currentText();
        QString especialista = ui->comboBox_especialista->currentText();
        QString fechaConsulta = ui->dateEdit_fechaConsulta->date().toString("yyyy-MM-dd");
        QTime hora_inicio = ui->timeEdit_hora->time();
        QTime hora_fin = hora_inicio.addSecs(7200); // cada cita tiene una duracion predeterminada de 2 horas
        QString motivo = ui->textEdit_motivo->text();
        //Indices de los comboBox para la especialidad y el especialista
        int indice_especialidad = ui->comboBox_especialidad->currentIndex();
        int indice_especialista = ui->comboBox_especialista->currentIndex();

        bool datos_validos = true;

        if (nombre.isEmpty()) {
            ui->lineEdit_nombrePaciente->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }
        if (cedula.isEmpty()) {
            ui->lineEdit_cedula->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }
        if (telefono.isEmpty() || telefono.size() < 11 || (!telefono.startsWith("0274") && !telefono.startsWith("0412") && !telefono.startsWith("0414") && !telefono.startsWith("0416") && !telefono.startsWith("0422") && !telefono.startsWith("0424") && !telefono.startsWith("0426"))) {
            ui->lineEdit_telefono->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }
        if (direccion.isEmpty()) {
            ui->lineEdit_direccion->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }
        if (correo.isEmpty() || !correo.endsWith("@gmail.com")) {
            ui->lineEdit_correo->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }
        if (indice_sexo == 0) {
            ui->comboBox_sexo->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }
        if (indice_especialidad == 0) {
            ui->comboBox_especialidad->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }
        if (indice_especialista == 0) {
            ui->comboBox_especialista->setStyleSheet("border: 2px solid red;");
            datos_validos = false;
        }

        if (!datos_validos) {
            QMessageBox::warning(this, "Error", "La informacion suministrada es invalida.");
            return;
        }

        //Validacion para evitar que dos citas se solapen
        QSqlQuery queryHora;
        queryHora.prepare("SELECT COUNT(*) FROM citas "
                          "WHERE especialista = :esp AND fecha = :fecha "
                          "AND (hora_inicio <= :fin AND hora_fin >= :inicio)");
        queryHora.bindValue(":esp", especialista);
        queryHora.bindValue(":fecha", fechaConsulta);
        queryHora.bindValue(":fin", hora_fin.toString("HH:mm"));
        queryHora.bindValue(":inicio", hora_inicio.toString("HH:mm"));

        if (queryHora.exec() && queryHora.next()) {
            int coincidencias = queryHora.value(0).toInt();
            if (coincidencias > 0) {
                QMessageBox::warning(this, "Error", "Lo sentimos, el horario solicitado ya está ocupado.");
                return;
            }
        }

        // Guardar paciente
        QSqlQuery queryPaciente;
        queryPaciente.prepare("INSERT OR REPLACE INTO pacientes (cedula, nombre, fecha_nac, sexo, numero_telefono, direccion, correo) "
                              "VALUES (:cedula, :nombre, :fecha_nac, :sexo, :numero_telefono, :direccion, :correo)");
        queryPaciente.bindValue(":cedula", cedula);
        queryPaciente.bindValue(":nombre", nombre);
        queryPaciente.bindValue(":fecha_nac", fechaNacimiento);
        queryPaciente.bindValue(":sexo", sexo);
        queryPaciente.bindValue(":numero_telefono", telefono);
        queryPaciente.bindValue(":direccion", direccion);
        queryPaciente.bindValue(":correo", correo);

        if (!queryPaciente.exec()) {
            QMessageBox::critical(this, "Error de Base de Datos", "No se pudo guardar el paciente:\n" + queryPaciente.lastError().text());
            return;
        }

        // Guardar cita
        QSqlQuery queryCita;
        queryCita.prepare("INSERT INTO citas (especialidad, especialista, fecha, hora_inicio, hora_fin, motivo, cedula_paciente) "
                          "VALUES (:especialidad, :especialista, :fecha, :hora_inicio, :hora_fin, :motivo, :cedula_paciente)");
        queryCita.bindValue(":especialidad", especialidad);
        queryCita.bindValue(":especialista", especialista);
        queryCita.bindValue(":fecha", fechaConsulta);
        queryCita.bindValue(":hora_inicio", hora_inicio.toString("HH:mm"));
        queryCita.bindValue(":hora_fin", hora_fin.toString("HH:mm"));
        queryCita.bindValue(":motivo", motivo);
        queryCita.bindValue(":cedula_paciente", cedula);

        if (!queryCita.exec()) {
            QMessageBox::critical(this, "Error de Base de Datos", "No se pudo guardar la cita:\n" + queryCita.lastError().text());
            return;
        }

        QString mensajeExito = QString("<b>Cita registrada correctamente</b><br><br>"
        "<b>Paciente:</b><br>"
        "&nbsp;&nbsp;Nombre: %1<br>"
        "&nbsp;&nbsp;Cédula: %2<br>"
        "&nbsp;&nbsp;Sexo: %3<br>"
        "&nbsp;&nbsp;Telefono: %4<br>"
        "&nbsp;&nbsp;Direccion: %5<br>"
        "&nbsp;&nbsp;Correo: %6<br><br>"
        "<b>Detalles de la Cita:</b><br>"
        "&nbsp;&nbsp;Especialidad: %7<br>"
        "&nbsp;&nbsp;Especialista: %8<br>"
        "&nbsp;&nbsp;Fecha: %9<br>"
        "&nbsp;&nbsp;Hora de Inicio: %10<br>"
        "&nbsp;&nbsp;Hora de Cierre: %11<br>"
        "&nbsp;&nbsp;Motivo: %12<br>")
        .arg(nombre, cedula, sexo, telefono, direccion, correo, especialidad, especialista, fechaConsulta, hora_inicio.toString("HH:mm"), hora_fin.toString("HH:mm"), motivo);
         QMessageBox::information(this, "Éxito", mensajeExito);

        // Limpiar campos después de guardar
        ui->lineEdit_nombrePaciente->clear();
        ui->lineEdit_cedula->clear();
        ui->lineEdit_telefono->clear();
        ui->lineEdit_direccion->clear();
        ui->lineEdit_correo->clear();
        ui->comboBox_sexo->setCurrentIndex(0);
        ui->comboBox_especialidad->setCurrentIndex(0);
        ui->comboBox_especialista->setCurrentIndex(0);
        ui->textEdit_motivo->clear();
        ui->dateEdit_fechaNacimiento->setDate(QDate(2000,1,1));
        ui->dateEdit_fechaConsulta->setDate(QDate::currentDate());
        ui->timeEdit_hora->setTime(QTime::currentTime());
        }
    }

    void MainWindow::mostrarPacienteDeFila(int fila, int columna)
    {
        if (columna == 0) {
            QString cedula = ui->tableWidget_citas->item(fila, 1)->text();
            QSqlQuery queryPaciente;
            queryPaciente.prepare("SELECT cedula, nombre, fecha_nac, sexo, numero_telefono, direccion, correo "
                          "FROM pacientes WHERE cedula = :cedula");
            queryPaciente.bindValue(":cedula", cedula);

            if (!queryPaciente.exec()) {
                QMessageBox::critical(this, "Error", "No se pudo consultar el paciente: " + queryPaciente.lastError().text());
                return;
            }
            if (queryPaciente.next()) {
                QString info_del_paciente;
                info_del_paciente += "Nombre: " + queryPaciente.value("nombre").toString() + "\n";
                info_del_paciente += "Cédula: " + queryPaciente.value("cedula").toString() + "\n";
                info_del_paciente += "Fecha de nacimiento: " + queryPaciente.value("fecha_nac").toString() + "\n";
                info_del_paciente += "Sexo: " + queryPaciente.value("sexo").toString() + "\n";
                info_del_paciente += "Teléfono: " + queryPaciente.value("numero_telefono").toString() + "\n";
                info_del_paciente += "Dirección: " + queryPaciente.value("direccion").toString() + "\n";
                info_del_paciente += "Correo: " + queryPaciente.value("correo").toString();

                QMessageBox::information(this, "Informacion del paciente", info_del_paciente);
            }
        }
    }

    void MainWindow::mostrarCitas_tablaCitas() {
        QString consulta = "SELECT p.nombre, p.cedula, c.especialidad, c.especialista, c.fecha, c.hora_inicio, c.hora_fin, c.motivo "
                                 "FROM pacientes p JOIN citas c ON p.cedula = c.cedula_paciente";

        QSqlQuery query;
        if (!query.exec(consulta)) {
            qDebug() << "Error: " << query.lastError().text();
        }

        ui->tablaCitas->setRowCount(0);
        int fila = 0;
        while (query.next()) {
            ui->tablaCitas->insertRow(fila);

            ui->tablaCitas->setItem(fila, 0, new QTableWidgetItem(query.value(0).toString())); // nombre
            ui->tablaCitas->setItem(fila, 1, new QTableWidgetItem(query.value(1).toString())); // cedula
            ui->tablaCitas->setItem(fila, 2, new QTableWidgetItem(query.value(2).toString())); // especialidad
            ui->tablaCitas->setItem(fila, 3, new QTableWidgetItem(query.value(3).toString())); // especialista
            ui->tablaCitas->setItem(fila, 4, new QTableWidgetItem(query.value(4).toString())); // fecha
            ui->tablaCitas->setItem(fila, 5, new QTableWidgetItem(query.value(5).toString())); // hora de inicio
            ui->tablaCitas->setItem(fila, 6, new QTableWidgetItem(query.value(6).toString())); // hora de cierre
            ui->tablaCitas->setItem(fila, 7, new QTableWidgetItem(query.value(7).toString())); // motivo
            fila++;
        }
    }

    void MainWindow::mostrarCitas_tableWidget_citas() {
        QString consulta = "SELECT p.nombre, p.cedula, c.especialidad, c.especialista, c.fecha, c.hora_inicio, c.hora_fin, c.motivo "
                           "FROM pacientes p JOIN citas c ON p.cedula = c.cedula_paciente";

        QSqlQuery query;
        if (!query.exec(consulta)) {
            qDebug() << "Error: " << query.lastError().text();
        }

        ui->tableWidget_citas->setRowCount(0);
        int fila = 0;
        while (query.next()) {
            ui->tableWidget_citas->insertRow(fila);

            ui->tableWidget_citas->setItem(fila, 0, new QTableWidgetItem(query.value(0).toString())); // nombre
            ui->tableWidget_citas->setItem(fila, 1, new QTableWidgetItem(query.value(1).toString())); // cedula
            ui->tableWidget_citas->setItem(fila, 2, new QTableWidgetItem(query.value(2).toString())); // especialidad
            ui->tableWidget_citas->setItem(fila, 3, new QTableWidgetItem(query.value(3).toString())); // especialista
            ui->tableWidget_citas->setItem(fila, 4, new QTableWidgetItem(query.value(4).toString())); // fecha
            ui->tableWidget_citas->setItem(fila, 5, new QTableWidgetItem(query.value(5).toString())); // hora de inicio
            ui->tableWidget_citas->setItem(fila, 6, new QTableWidgetItem(query.value(6).toString())); // hora de cierre
            ui->tableWidget_citas->setItem(fila, 7, new QTableWidgetItem(query.value(7).toString())); // motivo
            fila++;
        }
    }


