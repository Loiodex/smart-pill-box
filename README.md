# smart-pill-box

Smart Pill Box basato su ESP32.
Il sistema permette di ricordare all'utente tramite una sveglia di assumere i farmaci stipati al suo interno. Il dispositivo integra diversi tipi di sensori. Tramite il display e i pulsanti posti nella parte superiore permette all'utente di navigare fra le schermate disponibili e di modificare i dati relativi all'orario, alla sveglia e di attivare o disattivare quest'ultima.
I componenti utilizzati per sono:
- scheda ESP32 DevKit v1
- Display OLED SH1106 128X64 da 1,3 pollici 
- Real Time Clock DS3231
- Sensore Temperatura e Umidità DHT11
- Passive Buzzer
- 5 button switch

<img width="1180" height="598" alt="circuit" src="https://github.com/user-attachments/assets/75a8fdd2-ce33-40be-b2a6-09d926171c88" />

` i sensori utilizzati nel diagramma sono solo indicativi in quanto la piattaforma [wokwi
](wokwi.com) non dispone dei sensori utilizzati per il progetto. Fanno riferimento i collegamenti sull'ESP 32 e come sono stati collegati fra di loro i componenti. Nel progetto non è stata utilizzata una bread board, ma schede millefori con i componenti saldati su di esse`
Il dispositivo mostra diverse schermate:
 - 1 MENÙ
	 mostra l'orario, in alto a sinistra la temperatura e in alto a destra con la scritta ON o OFF se la sveglia è attiva o meno
 - 2 INFORMAZIONI
	 mostra le pagine disponibili
 - 3 SET ALARM
	 permette di modificare la sveglia e di impostarla
- 4 ON/OFF ALARM
	permette di disattivare o attivare la sveglia impostata
- 5 SET TIME
	permette di modificare l'orario
Quando la sveglia suona e o viene attivata/disattivata a schermo viene mostrato un messaggio per informare l'utente dell'azione.


![button_view](https://github.com/user-attachments/assets/64f7439a-8bb9-48f0-849d-54e623e2cb6b)

La smart pill box presenta nella parte superiore 5 pulsanti per gestire le varie schermate, l'orario e la sveglia. 
Nella parte frontale è presente il display che mostra le informazioni e al di sotto un cassetto dove riporre i farmaci da assumere.
I primi due pulsanti (quelli bianchi) permettono di scorrere fra le varie schermate presenti.
Il pulsante centrale (quello rosso) è il pulsante di "conferma":
- conferma la nuova sveglia se modificata
- conferma il nuovo orario se modificato
(se non viene premuto le modifiche apportate non verranno salvate)
- attiva o disattiva la sveglia se ci si trova nella schermata numero 4
- disattiva la sveglia se sta suonando
Gli ultimi due pulsanti ( quelli blu ) modificano, solo se ci si trova nelle rispettive schermate, l'orario e l'ora della sveglia.
- Nel caso dell'orario il primo pulsante blu ( quello di sinistra ) aumenta le ore di 1 , mentre il secondo ( quello di destra ) incrementa di 1 i minuti
- nel caso dell'ora di attivazione della sveglia il pulsante di sinistra diminuisce di 5 minuti l'orario impostato, mentre quello di destra lo incrementa.
