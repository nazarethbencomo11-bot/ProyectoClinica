#ifndef PERSONA_H
#define PERSONA_H

#include <QString>
#include <QDate>

class Persona {
public:
    Persona();
    virtual QString obtenerResumen() const = 0;
    virtual ~Persona() {}
};

class Paciente : public Persona {
private:
    QString nombre;
    QDate fecha_Nacimiento;
    QString cedula;
    QString numero_Telf;
    QString direccion;
    QString correo;

public:
    Paciente(QString nombre, QDate fecha_Nacimiento, QString cedula, QString numero_Telf, QString direccion, QString correo);
    QString obtenerResumen() const override;

};

class cita : public Persona
{
private:
    int id_paciente;
    QString especialidad;
    QString especialista;
    QDate fecha_cita;
    QString hora;
    QString motivo;
public:
    cita(int id_paciente, QString especialidad, QString especialista, QDate fecha_cita, QString hora, QString motivo);
    QString obtenerResumen() const override;
};

#endif // PERSONA_H
