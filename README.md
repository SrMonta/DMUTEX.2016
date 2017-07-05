# Segunda práctica individual de SSDD

Esta es la segunda práctica, en la que conseguí sacar un 7. Funciona perfectamente, solo hay que añadir una cosa que a mi no me dio tiempo.

Según la implementación que yo hice y según lo entendí, para comparar cual proceso es el que tiene prioridad en un lock, solo comparaba el valor del reloj correspondiente de cada proceso. *__ERROR__*. Hay que comparar todo el reloj. Es decir, lo conveniente es: cada vez que se haga un lock, salvaguardar el valor entero del reloj, y cuando llegue otra petición para la misma sección, comparamos el valor del reloj guardado con el que llega para saber si responder OK o no.

Si estas leyendo esto, es por que yo confío en ti o, en su defecto, alguien en quien yo confío confía a la vez en ti.

__La cuestión es:__ Creo firmemente en la libertad de la información, y más si el objetivo es aprender, como debería ser en una universidad. Obviamente, esto no es código profesional, ninguna empresa se verá perjudicada por ello. Pero siguen existiendo correctores de prácticas, ya que desgraciadamente, sigue existiendo gente que copia en vez de basarse en otras cosas. Si subo esto aquí es por que confío en que aquel que lo esté leyendo, lo usará legítimamente. Llámame idealista. Pero... ¿Entonces es un repositorio público?. No. De nuevo, confío en que __TÚ__ usarás esto correctamente. El problema es que si hago este repositorio público, correremos el riesgo de que algún idiota copie y pegue este código, y empiecen a saltar copias por todos lados. A mí me da igual, yo ya tengo aprobado esto, pero imagino que a tí no.

Pues eso, úsalo a conciencia, entiende lo que está escrito, y úsalo para desarrollar tu propio código.

Si ves que no entiendes algo, en mi perfil de GitHub está mi correo, siéntete libre de usarlo. No SPAM, please.

Un saludo, y buen código!
