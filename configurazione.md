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
**N.B.** in ```ip route``` nell'ip dell'host bisogna ricordarsi ```/32``` (altrimenti se minore di 32 indica una subnet), inoltre in alcuni casi può esserci bisogno di specificare l'interfaccia:
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
