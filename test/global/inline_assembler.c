#include <stdio.h>

int main(void)
{
  int foo = 5;
  int bar = 4;

  /* Hier beginnt der Inline-Assembler Abschnitt in AT&T-Syntax */
  __asm__ (
      "add %1, %0\n\t" /* Addiert den Wert von Operand %1 zum Wert von Operand %0. */ {{A}}
{{B}}
      "inc %0"         /* Erhöht den Wert von Operand %0 um 1. */

      /* Definition der Nebenbedingungen:
       * Diese weisen den C-Variablen der Reihe nach aufzählend einen für den Inline-Assemblercode nutzbaren Operanden
       * zu und teilen dem Compiler mit, auf welche Weise (lesend und/oder schreibend) dieser im Inline-Assemblercode
       * verwendet werden kann und auf welche Register dieser beschränkt ist. Dadurch wird eine korrekte und effiziente
       * Übergabe der Variablenwerte in und aus dem Assemblercodeabschnitt gewährleistet. */
      : "+r" (bar) /* Gibt an, dass die Variable bar sowohl gelesen als auch beschrieben ('+') wird und deren Wert in
                    * ein allgemeines Register ('r') zu platzieren ist.
                    * Als erster Operand wird er im Assemblercodeabschnitt unter der Bezeichnung %0 genutzt. */
      : "g" (foo)  /* Beschränkt die Verwendung der Variable foo nur zum Lesen. Sie kann auf beliebige Weise ('g', im
                    * Speicher, in einem Register oder als Direktwert) an den Assemblerteil übergeben werden.
                    * Als zweiter Operand wird er im Assemblercodeabschnitt unter der Bezeichnung %1 genutzt */
      : "cc"       /* Gibt an, dass die Statusanzeige (durch die Befehle add und inc) verändert wurde. */
  );

  /* Hier geht's mit C Code weiter. */
  printf("Ergebnis: %i\n", bar);
  return 0;
}


/*
 * check-name: test inline assembler
 * A != B
 */

