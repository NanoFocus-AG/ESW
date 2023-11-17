 

# Ticket #6123

Es ist ein Export konform zu Mahr Profildateien zu schreiben. Dateiendung *.prf

Im Anhang eine Beispieldatei (989439604188006112011190730701035751_Kontur_1_filtered.txt). Nach erfolgreicher Implementierung in den Trunk muss die Datei
auf 8.6 zurück portiert werden. Weitere Informationen folgen.

Nach Reverse-Engineering der Textdatei:

    Die Daten sind im X-/Y-/Z-Format abgelegt
    Spalte 1: Fortlaufende Nummer, Anschließendes Trennzeichen ist '='
    Spalte 2: Tischposition X-Achse zu jedem Messwert in mm, Dezimaltrennzeichen = "."
    Spalte 3: Tischposition Y-Achse zu jedem Messwert
    Spalte 4: Tischposition Z-Achse+Höhenwert des Sensors in mm
    Spalte 5: Wahrscheinlich weitere Achse (Hier immer 0.000000000)
    Spalte 6: 0


 