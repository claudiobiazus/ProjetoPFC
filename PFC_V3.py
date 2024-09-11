import serial
from PySimpleGUI import PySimpleGUI as sg

# Loop para Conexão com Arduino
while True:
    try:
        arduino = serial.Serial('COM7', 19200)
        print("Arduino conectado")
        break
    except:
        print("Arduino Offline")
        pass

# Criação da interface
# Tema para a tela
sg.theme('DarkBlue')
# Layout da tela
layout =  [
    [sg.Text("PERSONAL FOOD COMPUTER", size=(30, 0), justification="center", font=("Helvetica", 20, "bold"))],
    [sg.Text("", key="-HORA-")],
    [sg.Text("")],
    [sg.Text("ILUMINAÇÃO DA PLANTA", size=(30, 0), justification="center"), sg.Text("ILUMINAÇÃO AUXILIAR", size=(30, 0), justification="center"), sg.Text("VENTILAÇÃO", size=(30, 0), justification="center")],
    [sg.Text("", key="-LUZ1-", size=(30, 0), justification="center"), sg.Text("", key="-LUZ2-", size=(30, 0), justification="center"), sg.Text("", key="-VENT-", size=(30, 0), justification="center")],
    [sg.Text("")],
    [sg.Text("")],
    [sg.Text("PARÂMETROS DO AR", font=("Helvetica", 15, "bold"))],
    [sg.Text("")],
    [sg.Text("TEMPERATURA DO AR", size=(30, 0), justification="center"), sg.Text("UMIDADE DO AR", size=(30, 0), justification="center"), sg.Text("NÍVEL DE CO2", size=(30, 0), justification="center")],
    [sg.Text("", key="-TEMP1-", size=(30, 0), justification="center"), sg.Text("", key="-UMID-", size=(30, 0), justification="center"), sg.Text("", key="-GAS-", size=(30, 0), justification="center")],
    [sg.Text("")],
    [sg.Text("PARÂMETROS DA ÁGUA", font=("Helvetica", 15, "bold"))],
    [sg.Text("")],
    [sg.Text("TEMPERATURA DA ÁGUA", size=(30, 0), justification="center"), sg.Text("CONDUTIVIDADE ELÉTRICA", size=(30, 0), justification="center"), sg.Text("POTENCIAL HIDROGENIÔNICO", size=(30, 0), justification="center")],
    [sg.Text("", key="-TEMP2-", size=(30, 0), justification="center"), sg.Text("", key="-COND-", size=(30, 0), justification="center"), sg.Text("", key="-PH-", size=(30, 0), justification="center")],
    [sg.Text("")],
    [sg.Text("AVISOS:", font=("Helvetica", 15, "bold"))],
    [sg.Text("", key="-INFO-", text_color="red",justification="center" ,font=("Helvetica", 13, "bold"))],
    [sg.Text("")],
    []
]
window = sg.Window('Personal Food Computer', layout, element_justification="center", finalize=True)
def verificar():
    hour = int(arduino.readline().decode().strip())
    min = int(arduino.readline().decode().strip())
    tempA = float(arduino.readline().decode().strip())
    co2 = float(arduino.readline().decode().strip())
    umid = float(arduino.readline().decode().strip())
    tempB = float(arduino.readline().decode().strip())
    tds = float(arduino.readline().decode().strip())
    ph = float(arduino.readline().decode().strip())
    sTempA = int(arduino.readline().decode().strip())
    sTDS = int(arduino.readline().decode().strip())
    sCO2 = int(arduino.readline().decode().strip())
    sUmi = int(arduino.readline().decode().strip())
    sTempB = int(arduino.readline().decode().strip())
    sCooler = int(arduino.readline().decode().strip())
    sLuz = int(arduino.readline().decode().strip())
    arduino.flush()
    print("LEITURA CONCLUIDA")

    return hour, min, tempA, co2, umid, tempB, tds, ph, sTempA, sTDS, sCO2, sUmi, sTempB, sCooler, sLuz

def status(t1, tds, co2, umid, t2, co, luz):
    if t1 == 1:
        tp1 = "\nÁGUA MUITO QUENTE!"
    elif t1 == -1:
        tp1 = "\nÁGUA MUITO FRIA"
    else:
        tp1 = "\n"

    if tds == 1:
        td1 = "\nNIVEL DE CE ELEVADO"
    elif tds == -1:
        td1 = "\nNÍVEL DE CE MUITO BAXO"
    else:
        td1 = "\n"

    if co2 == 1:
        co1 = "\nNÍVEL DE CO2 ELEVADO"
    elif co2 == -1:
        co1 = "\nNÍVEL DE CO2 BAIXO"
    else:
        co1 = "\n"

    if umid == 1:
        um = "\nNÍVEL DE UMIDADE DO AR ELEVADO"
    elif umid == -1:
        um = "\nNIVEL DE UMIDADE DO AR BAIXO"
    else:
        um = "\n"

    if t2 == 1:
        tp2 = "\nTEMPERATURA DO AR ELEVADA"
    elif t2 == -1:
        tp2 = "\nTEMPERATURA DO AR BAIXA"
    else:
        tp2 = "\n"

    if co == 1:
        cl1 = "LIGADO"
    else:
        cl1 = "DESLIGADO"

    if luz == 1:
        l1 = "LIGADO"
        l2 = "LIGADO"
    elif luz == -1:
        l1 = "DESLIGADO"
        l2 = "DESLIGADO"
    else:
        l1 = "DESLIGAO"
        l2 = "LIGADO"

    return tp1, td1, co1, um, tp2, cl1, l1, l2

def cor(a, b, c):
    if a == "LIGADO":
        c1 = "green"
    else:
        c1 = "red"

    if b == "LIGADO":
        c2 = "green"
    else:
        c2 = "red"

    if c == "LIGADO":
        c3 = "green"
    else:
        c3 = "red"

    return c1, c2, c3

while True:
    event, values = window.read(timeout=1000)

    if event == sg.WIN_CLOSED:
        break
    else:
        window.refresh()

    h, m, t1, c, u, t2, ce, p, s1, s2, s3, s4, s5, s6, s7 = verificar()
    a1, a2, a3, a4, a5, cooler, luz1, luz2 = status(s1, s2, s3, s4, s5, s6, s7)
    cor1, cor2, cor3 = cor(luz1, luz2, cooler)
    info = a1 + a2 + a3 + a4 + a5 + "\n"
    hora = f"{h}" + ":" + f"{m}"

    window["-HORA-"].update(f"{hora}h")

    window["-LUZ1-"].update(f"{luz1}", text_color=cor1)
    window["-LUZ2-"].update(f"{luz2}", text_color=cor2)
    window["-VENT-"].update(f"{cooler}", text_color=cor3)

    window["-TEMP1-"].update(f"{t2}ºC")
    window["-UMID-"].update(f"{u}%")
    window["-GAS-"].update(f"{c} ppm")

    window["-TEMP2-"].update(f"{t1}ºC")
    window["-COND-"].update(f"{ce} ce")
    window["-PH-"].update(f"{p} pH")

    window["-INFO-"].update(f"{info}")
window.close()