# Satellite autonome client

PRESENTATION GENERALE DU CONCEPT DE SATELLITE AUTONOME

/!\ ATTENTION : Ce programme est en cours de développement, il n'est donc pas opérationnel dans son ensemble.

Je propose ici une nouvelle approche du concept des satellites devellopé par quelques membres de Locoduino, qui avait été présentés au salon du modélisme ferroviaire d’Orléans en 2018 et qui a fait l’objet d’une série d’articles.

https://locoduino.org/spip.php?article243 (et suivants)

Plusieurs années après, ce projet pourtant très riche et innovant n’a pas vraiment trouvé sa place sur les réseaux ferroviaires ce qui était bien sûr son ambition.

La première et principale raison selon moi est que ces satellites ne s’interfaçaient avec aucun gestionnaire existant (JMRI, RocRail…) et que les tentatives de mise au point de gestionnaires « maison » se sont révélées compliquées.

Ayant une très forte volonté de pouvoir implanter ce concept de satellite sur mon réseau, je me suis intéressé à voir comment je pouvais réaliser cet interfaçage avec JMRI et Rocrail, deux logiciels gratuits de gestion de réseau.

Mais je me suis assez vite rendu compte que le problème était finalement plus du côté de ces logiciels, eux-mêmes fastidieux à programmer pour la gestion de réseau. Il fallait de plus réaliser au préalable une description précise du réseau pour en avoir le « mapping ». Cette description devenant obsolète dès que le réseau est modifié.

Pour relancer ce concept de satellites, je voulais en conserver l’esprit mais il fallait pouvoir proposer à la majorité des modélistes un système simple et efficace ce qui est l’esprit et la motivation de Locoduino. C’est en travaillant tout d’abord sur une solution de description de réseau qu’a émergé progressivement cette approche de « satellites autonomes » que je propose.

Je me suis interrogé sur la question de savoir si un satellite, associé à un canton, ne pouvait pas assurer toutes les fonctions liées à l’état des équipements de voie (aiguilles), à l’occupation des cantons environnants, à la signalisation, aux commandes à adresser aux locomotives pour adapter leur comportement (vitesse, klaxon etc…) ?

Avec l’adoption du bus CAN (qui présente la particularité de pouvoir échanger des messages « broadcasts »), chaque satellite peut connaître en temps réel l’état des tous les cantons de son environnement. Il devient donc capable de prendre seul les décisions appropriées à la gestion de réseau.

Voilà résumé ce qu’il y a derrière l’expression « satellite autonome ». Un satellite sans gestion centralisée et surtout sans paramétrage fastidieux.

Avec cette nouvelle approche du concept de satellites, je voulais profiter pour corriger quelques défauts, selon moi, inhérents à la version 1.


Parmi les défauts à corriger que j’avais identifié sur la version 1 :

1.	Chaque satellite devait être programmé avec un identifiant spécifique et « en dur ».
2.	Le système de signalisation (feux lumineux) propre à chaque satellite devait lui aussi être programmé « en dur ».
3.	De la même manière, le réglage des valeurs des butées de servomoteurs d’aiguille n’avait pas trouvé de solution simple. Donc nécessitait là aussi d’être programmé « en dur »
4.	Enfin, la messagerie adoptée limitait à 16 le nombre de satellites ce qui est trop peu pour la plupart des réseaux et la messagerie limitait aussi certaines utilisations comme la détection par RFID par exemple.

Les points 1, 2 et 3 en particulier obligeaient à adapter chaque programme avant son téléversement, les mises à jour devenaient compliquées et les risques d’erreurs importants.

Avec cette nouvelle version, tous les programmes à télécharger sont absolument identiques pour tous les satellites. Les opérations de maintenance et de mise à jour sont considérablement simplifiées.

