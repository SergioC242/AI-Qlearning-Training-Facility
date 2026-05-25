Este proyecto entrena la IA de Qlearning de blackjack para este proyecto https://github.com/UCM-FDI-DISIA/CookingMonsters

Trata de imitar lo mas fielmente posible el comportamiento de el juego original para poder hacer miles de entrenamientos para crear la QTable


La IA utiliza un algoritmo de aprendizaje por refuerzo llamado Q-Learning. En lugar de seguir reglas predefinidas, la IA aprende qué acciones conducen a mejores resultados recibiendo recompensas basadas en los resultados de los encuentros de combate a través de repetidas partidas.

Funcionamiento:

Lo que sabe la IA:
- La vida del jugador y el enemigo
- la puntuación
- Probabilidad de pasarse de 21
- que dados tiene

Recompensas en el aprendizaje:
- Ganar ronda +1
- Perder ronda -1
- Ganar Partida +10
- Perder Partida -10
- Empatar Partida 0
