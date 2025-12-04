#include "persona.h"
#include <QSqlQuery>
#include <QSqlError>

Persona::Persona() {}

Paciente::Paciente(QString nombre, QDate fecha_Nacimiento, QString cedula, QString numero_Telf, QString direccion, QString correo)
    : nombre(nombre), fecha_Nacimiento(fecha_Nacimiento), cedula(cedula), numero_Telf(numero_Telf), direccion(direccion), correo(correo) {}

QString Paciente::obtenerResumen() const {
    return QString("Nombre: %1\nFecha de Nacimiento: %2\nCedula: %3\nNumero de telefono: %4\nDireccion: %5\nCorreo Electronico: %6")
    .arg(nombre).arg(fecha_Nacimiento.toString("yyyy-MM-dd")).arg(cedula).arg(numero_Telf).arg(direccion).arg(correo);
}

cita::cita(int id_paciente, QString especialidad, QString especialista, QDate fecha_cita, QString hora, QString motivo) : id_paciente(id_paciente), especialidad(especialidad), especialista(especialista), fecha_cita(fecha_cita), hora(hora), motivo(motivo) {}

QString cita::obtenerResumen() const {
    return QString("Id de paciente: %1\nEspecialidad: %2\nEspecialista: %3\nFecha de consulta: %4\nHora de consulta: %5\nMotivo: %6")
    .arg(id_paciente).arg(especialidad).arg(especialista).arg(fecha_cita.toString("yyyy-MM-dd")).arg(hora).arg(motivo);
}

/*QString Persona::obtenerResumen() const {
    return QString("Nombre: %1\n").arg(nombre);
}*/
