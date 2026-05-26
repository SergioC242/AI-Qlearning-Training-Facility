Este proyecto entrena la IA de Qlearning de blackjack para este proyecto https://github.com/SergioC242/IAV26-CrocheTrigo

Trata de imitar lo mas fielmente posible el comportamiento de el juego original para poder hacer miles de entrenamientos para crear la QTable


La IA utiliza un algoritmo de aprendizaje por refuerzo llamado Q-Learning. En lugar de seguir reglas predefinidas, la IA aprende qué acciones conducen a mejores resultados recibiendo recompensas basadas en los resultados de los encuentros de combate a través de repetidas partidas. El jugador es el que es controlado por una IA

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


Archivos y funciones:
Las decisiones de la IA se almacenan en un archivo llamado qtable.txt. Estas son entrenadas, procesadas y almacenadas por AIplayer.h

Las variables que determinan como aprende la IA son Alpha, Gamma y Epsilon.

- Alpha: es el valor que determina cuanto peso se le da a las nuevas recompensas. Si es baja aprende lento pero previene que se generen recompensas erroneas (si un caso que 99% es malo pero le sale bien le da poca importancia ya que para que se aprecie tendria que salirle bien repetidas veces)
- Gamma: es el valor que determina cuanto peso en la decision tiene poder ganar ahora o en el futuro. Si es baja siempre busca ganar aun tan poco sea de inmediato pero si es alta busca ganar en el futuro.
- Epsilon: es el valor que determina la probabilidad de que la IA haga una decision aleatoria en vez de seguir la mejor conocida. (Esto es conocido como ε-greedy)

Metodos:
- chooseAction: Elige una opcion, generando un numero random dependiendo de si cae en el rango de Epsilon o no elige una accion legal aleatoria o la mejor conocida.
- rewardBust: Se le aplica una recompensa negativa a la IA si se pasa de 21. (actualmente -5)
- RewardRoundEnd: +1 o -1 dependiendo de si se gana o se pierde la ronda.
- RewardGameEnd: +10 o -10 dependiendo de si se gana o se pierde la partida.
- RewardDraw: +0
- clearHistory: Limpia la lista de acciones tomadas por la IA sin antes guardas sus resultados. RewardDraw la usa para no dar una recompensa.
- Save QTable: Guarda la tabla de decisiones de la IA. Para eso utiliza un formato de: Numero que tiene codificado la información en el momento de la decisión, Numero de accionn tomada, Recompensa recibida.
- loadQTable: Carga la tabla de decisiones de la IA.
- getQValue: Mira las distintas acciones y recompensas para X situacion y Acción.
- Set y get Epsilon: No usados actualmente. Se pueden usar para generar aprendizajes mas optimos ya que cuando se determina que la IA ya ha aprendido lo suficiente se puede bajar para que pare de utilizar opciones aleatorias y busque solo refinar los valores de recompensa de las acciones optimas.
- maxQ: Devuelve la mayor recompensa para un estado.
- applyReward: Aplica las recompensas a las acciones tomadas y borra la lista de acciones tomadas.


Combat: LLeva a cabo las rondas y llama a las funciones de la IA para tomar decisiones.

CombatState: Se encarga de que el estado actual este encodeado en el formato de la tabla de decisiones.

Deck y Die: Guardan y manejan las cartas y los dados.

Enemy: Sigue las reglas de una IA simple contra la que se enfrenta la IA del jugador.
