# Configurazione reti marionnet

## Configurazione di un'interfaccia
Le interfacce possono essere tirate su/giù manualmente con ```ifup -a``` e ```ifdown -a``` seguendo il file di configurazione ```/etc/network/interfaces```, l'opzione ```-a``` specifica di fare riferimento a tutte le interfacce segnate nel file da ```auto```.

```
auto <nome_int>
interface <nome_int> inet <static/dhcp>
    <opzioni>
```

Opzioni:
- ```address```: specifica l'indirizzo IP, obbligatorio se si configura l'interfaccia come static, può comprendere anche la netmask, es. ```192.168.1.1/24```
- ```netmask```: specifica la netmask, es. ```255.255.0.0```
- ```gateway```: specifica l'indirizzo IP del default gateway
- ```hwaddress ether <mac>```: specifica il mac address dell'interfaccia
- ```post-up```: permette di specificare comandi da eseguire ogni volta che viene tirata su l'interfaccia
- ```vlan-raw-device```: **non usata a lezione, ma la wiki di debian dice di usarla**, se l'interfaccia è un'interfaccia virtuale su una vlan, serve per specificare quale interfaccia fisica deve utilizzare, ad esempio per avere un'interfaccia virtuale su vlan 222 sull'interfaccia fisica eth0:

```
auto eth0.222
iface eth0.222 inet static
        address 10.10.10.1/24
        vlan-raw-device eth0
```

Per maggiori info: https://wiki.debian.org/it/NetworkConfiguration.

## Configurazione dei nomi degli host
Modificando il file ```/etc/hosts``` aggiungendo righe:

```
<IP>    <hostname>
```

## Configurazione delle VLAN
Per configurare le vlan, bisogna modificare il file di configurazione degli switch usando i seguenti comandi:
- ```vlan/create <vlan_n>```: crea una vlan con numero ```<vlan_n>```
- ```port/setvlan <port_n> <vlan_n>```: assegna alla porta ```<port_n>``` la vlan ```<vlan_n>``` => **UNTAGGED**
- ```vlan/addport <vlan_n> <port_n>```: aggiunge la porta ```<port_n>``` alla vlan ```<vlan_n>``` => **TAGGED**

Esempio di configurazione per 2 vlan 10 e 20, port based rispettivamente sulle porte 1 e 2 ed entrambe tramite tag sulla porta 2:

```
vlan/create 10
vlan/create 20

port/setvlan 1 10
port/setvlan 2 20

vlan/addport 10 3
vlan/addport 20 3
```

## Routing
### Lato gateway
Sul gateway è fondamentale abilitare l'ip forwarding, per controllare se abilitato:
```
sysctl net.ipv4.ip_forward
```
Per abilitarlo:
```
sysctl -w net.ipv4.ip_forward=1
```
**N.B. all'avvio viene rimesso il valore di default, ovvero 0**, per evitare ciò si modifica il file ```/etc/sysctl.conf.```, che verrà poi caricato in automatico all'avvio, oppure manualmente con:
```
sysctl -p /etc/sysctl.conf
```
### Lato "client"
Ai client non serve abilitare l'ip forwarding, serve però settare il gateway/regole di routing.

Per visualizzare le tabelle di routing:
```
route -n
```
Per svuotare le tabelle di routing:
```
ip route flush table main
```

Per aggiungere regole, si possono utilizzare sia il comando ```route``` che ```ip route``` (sotto mostrati entrambi), le loro modifiche non sono permanenti quindi vanno aggiunti in ```post-up``` in ```/etc/network/interfaces```.

**N.B.** delle volte se si modifica la tabella di routing, soprattuto con configurazioni sbagliate, potrebbe essere che non si riesce più a tirare su le interfacce, nè svuotando le tabelle, nè con ```ifdown -a``` ```ifup -a``` e neanche con ```service networking restart```, in tal caso serve riavviare la macchina.

Le regole possono essere impostate in 3 modi:
1) _**Verso un host**_:
```
route add -host <host_ip> gw <gateway_ip>
```
```
ip route add <host_ip>/32 via <gateway_ip>
```
**N.B.** in ```ip route``` nell'ip dell'host bisogna ricordarsi ```/32``` (altrimenti se minore di 32 indica una subnet), inoltre in alcuni casi può esserci bisogno di specificare l'interfaccia, soprattutto se **l'host di destinazione è nella stessa LAN ma ha una subnet diversa, es. per i gateway**:
```
ip route add <host_ip>/32 dev <ethX>
```
1) _**Verso una subnet**_
```
route add -net <netid> netmask <netmask> gw <gateway_ip>
```
```
ip route add <netid>/<n> via <gateway_ip>
```
1) _**Impostando il gateway di default**_
```
route add default gw <gateway_ip>
```
```
ip route add default via <gateway_ip>
```
Oppure settando l'opzione ```gateway``` dell'interfaccia in ```/etc/network/interfaces```.

## DHCP/DNS
Per DHCP/DNS si usa il servizio **dnsmasq**, che si abilita all'avvio con
```
systemctl enable dnsmasq
```
e può essere avviato/riavviato manualmente con
```
service dnsmasq <start/restart>
```
dnsmasq è configurato tramite il file ```/etc/dnsmasq.conf``` con i seguenti parametri:
- ```no-resolv```: non cerca su altri nameservers
- ```read-ethers```: legge i contenuti di ```/etc/ethers```, che è uno dei modi per configurare degli ip statici, aggiungendo righe:
    ```
    <mac>   <ip>
    ```
- ```interface=<ethX>```: usa l'interfaccia ```ethX``` per il DHCP
- ```domain=<...>```: imposta il dominio
- ```dhcp-option=<option>```: opzioni del dhcp, le più importanti:
  1. ```<3/option:router>,<gateway_ip>```: imposta l'ip del gateway dei client DHCP, **non setta il suo** 
  2. ```<6/option:dns-server>,<dns_ip>```: imposta l'ip del DNS dei client DHCP, se il server fa anche da DNS (quindi mette il proprio ip come ```dns_ip```), allora risolverà gli hostname in base alla configurazione statica e in base al file ```/etc/hosts``` 

    Es. così si setta il gateway a ```10.10.10.254``` e il dns a ```8.8.8.8```:
    ```
    dhcp-option=option:router,10.10.10.254
    dhcp-option=option:dns-server,8.8.8.8
    ```
- ```dhcp-range=<ip_low>,<ip_high>,<time>```, assegna il range degli ip assegnabili dal dhcp, da ```ip_low``` a ```ip_high``` inclusi, con lease time ```time``` (es. ```1h```)
- ```dhcp-host=<mac>,<hostname>,<ip>,<time>```, assegna staticamente ```ip``` a ```mac```, dandogli anche l'hostname con lease time ```time```, questa è un'alternativa ad usare i file ```/etc/ethers``` e ```/etc/hosts```
- ```address=/<addr>/<ip>```, sovrascrive l'indirizzo di ```ip``` con ```addr```, alternativa al file ```/etc/hosts```

In presenza di VLAN, bisogna aggiungere dei parametri a:
- ```dhcp-range```, si aggiunge come _primo_ parametro ```set:vlan<id>```
- ```dhcp-option```, si aggiunge come _primo_ parametro ```tag:vlan<id>```
- ```dhcp-host```, si aggiunge come _ultimo_ parametro ```set:vlan<id>```

es. di configurazione con VLAN:
```
no-resolv
read-ethers
interface=eth0.10
domain=reti.org
dhcp-range=set:vlan10,10.0.1.2,10.0.1.125,24h
dhcp-option=tag:vlan10,option:netmask,255.255.255.128
dhcp-option=tag:vlan10,option:router,10.0.1.126
dhcp-option=tag:vlan10,option:dns-server,10.0.1.126
dhcp-host=02:04:06:11:11:22,H1,10.0.1.1,set:vlan10
```

## Traffic Shaping
Se è necessario semplicemento limitare la banda in uscita su un host, si può implementare senza classi con un tbf
```
tc qdisc add dev <ethX> root tbf rate <rate> latency <latency> burst <burst>
```

```rate```, ```latency``` e ```burst``` hanno le unità di misura, se non specificati di solito:
- ```latency``` è sulle decine di millisecondi
- ```burst``` è sulle migliaia (10k, 15k, 1539...)

Se invece è necessario limitare la banda in uscita in base al destinatario, allora bisogna usare qdisc classful, es:
```
tc qdisc add dev <ethX> root handle 1: htb default 20
tc class add dev <ethX> parent 1: classid 1:1 htb rate 100Mbit burst 15k
tc class add dev <ethX> parent 1:1 classid 1:10 htb rate <custom_rate> burst 15k
tc class add dev <ethX> parent 1:1 classid 1:20 htb rate <default_rate> burst 15k
tc qdisc add dev <ethX> parent 1:10 handle 10: pfifo limit 50
tc qdisc add dev <ethX> parent 1:20 handle 20: pfifo limit 50
tc filter add dev <ethX> protocol ip parent 1:0 prio 1 u32 match ip dst <ip_slow> flowid 1:10
```

In questo caso, sull'interfaccia ```ethX```, il traffico verso ```ip_slow``` viene mandato dal dilter verso la classe 1:10 con un rate custom ```custom_rate```, il resto del traffico viene invece mandato verso la classe 1:20 con il rate ```default_rate```.

## Firewall
Si utilizza ```iptables```, la sua configurazione non viene memorizzata tra un avvio e l'altro, quindi si possono usare:
```
iptables-save > <file>
```
```
iptables-restore < <file>
```

Per visualizzare le regole di una tabella:
```
iptables -t <tabella> -L -v -n
```

Per eliminarela regola ```n```, iniziando a contare da 1, di una catena:
```
iptables -t <tabella> -D <catena> <n>
```

Per eliminare tutte le regole di una catena:
```
iptables -t <tabella> -F <catena>
```

Per il firewall si opera sulla tabella ```filter```, essa contiene 3 catene:
1. ```INPUT```, regole per i pacchetti che hanno come destinazione questo host
2. ```FORWARD```, regole per i pacchetti in transito
3. ```OUTPUT```, regole per i pacchetti in uscita da questo host

Per ogni catena, si può impostare una policy di default:
```
iptables -t <tabella> -P <catena> <ACCEPT/DROP>
```
es. per bloccare tutto i pacchetti in transito su questo host:
```
iptables -t filter -P FORWARD DROP
```
Si possono poi aggiungere ulteriori regole alle catene tramite:
```
iptables -t filter -A <catena> {opzioni} -j <policy>
```

le ```{opzioni}``` possibili sono:
- ```-p <protocol>```, imposta il protocollo, può essere ```tcp```, ```udp```, ```icmp```,...
- ```-i <ethX>```/```-o <ethX>```, imposta l'interfaccia d'ingresso/uscita
- ```-s <ip>```/```-d <ip>```, imposta l'ip/subnet della sorgente/destinazione
- ```--sport <port>```/```--dport <port>```, imposta la porta in entrata/uscita, se sono porte legate a protocolli comuni spesso si può mettere il nome del protocollo al posto del numero di porta
- ```-m state --state <stato>```, permette di fare regole dinamiche che dipendono dallo stato della connessione, una regola può contenere più stati e possono essere:
    1. ```NEW```, fa riferimento a pacchetti che aprono la connessione, se si fanno regole dinamiche è **fondamentale** che da client verso server siano ammessi i ```NEW```
    2. ```ESTABLISHED```, fa riferimento a pacchetti di connessioni già aperte in precedenza
    3. ```RELATED```, fa riferimento a pacchetti legati a connessioni già stabilite, serve in alcuni casi per protocolli tipo ```icmp```, ```ftp``` (nella regola che usa come porta sorgente dal server ```ftp-data```),...

Infine la ```policy``` determina cosa bisogna fare ai pacchetti che soddisfano ```{opzioni}```:
- ```ACCEPT```
- ```DROP```
- ```REJECT```, come ```DROP``` ma invia anche un messaggio a chi ha inviato il pacchetto droppato
- ```QUEUE```, rende il pacchetto disponibile per elaborazioni in user space
- ```LOG```, logga informazioni relative al pacchetto

Esempio, per abilitare le connessioni verso un server ```<ip_server>``` di un protocollo sulla porta ```<port>``` passando per l'interfaccia esterna ```<eth_ext>``` e l'interfaccia interna ```<eth_int>```:

client => server
```
iptables -A FORWARD -p <protocol> --dport <port> -i <eth_ext> -o <eth_int> -d <ip_server> -m state --state NEW,ESTABLISHED -j ACCEPT
```

server => client
```
iptables -A FORWARD -p <protocol> --sport <port> -i <eth_int> -o <eth_ext> -s <ip_server> -m state --state ESTABLISHED -j ACCEPT
```

## NAT
Si usa sempre ```iptables``` sulla tabella ```nat``` con 3 catene:
1. ```PREROUTING```, permette di modificare **l'ip di destinazione** dei pacchetti provenienti dall'esterno
2. ```OUTPUT```, permette di modificare **l'ip di destinazione** dei pecchetti **generati localmente**
3. ```POSTROUTING```, permette di modificare **l'ip sorgente** di tutti i pacchetti in uscita

Per aggiungere regole di NAT:
```
iptables -t nat -A <catena> {opzioni} -j <policy>
```

Le ```{opzioni}``` sono le stesse del firewall.

Le policy possibili sono:
- ```SNAT --to-source <ip:porta>```, modifica ip e porta sorgenti del pacchetto
- ```MASQUERADE```, come ```SNAT```, però sostituisce l'ip sorgente con quello del interfaccia dell'host che fa NAT da cui uscirà, serve in caso l'ip dell'host che fa NAT sia dinamico
- ```DNAT --to-destination <ip:porta>```, modifica ip e porta di destinazione di un pacchetto
- ```REDIRECT```, come ```DNAT```, però sostituisce l'ip di destinazione con quello dell'host che fa NAT

Esempi:
1. Abilitare il NAT per permettere agli host sotto NAT di comunicare con l'esterno:
```
iptables -t nat -A POSTROUTING -o <ethX> -j SNAT --to-source <ip_pubblico>
```
dove ```<ethX>``` è l'interfaccia pubblica e ```<ip_pubblico>``` è l'ip pubblico, in caso esso sia dinamico si usa:
```
iptables -t nat -A POSTROUTING  -o <ethX> -j MASQUERADE
```
2. Esporre un servizio su un host con indirizzo ```<ip_locale>``` sulla porta ```<port>``` e protocollo ```<protocol>```:
```
iptables -t nat -A PREROUTING -p <protocol> -d <ip_pubblico> --dport <port> -j DNAT --to-destination <ip_locale>
```
3. Fare un Transparent Proxy che redirige tutti i pacchetti ```tcp``` verso la porta ```80``` alla porta ```8080``` dell'host ```192.168.1.1```:
```
iptables -t nat -A PREROUTING -p tcp -i <ethX> --dport 80 -j DNAT --to-destination 192.168.1.1:8080
```
Se il proxy si trova sull'host che fa NAT allora si può usare ```REDIRECT```:
```
iptables -t nat -A PREROUTING -p tcp -i <ethX> --dport 80 -j REDIRECT --to-port 8080
```