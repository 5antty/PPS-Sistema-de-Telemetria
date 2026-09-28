## Configuracion WiFi
Por defecto el gateway esta en modo Access Point, el cual nos permite conectarnos a su red propia para acceder a su configuracion a traves de un navegador accediendo a la url con su ip por defecto, que es la 192.168.230.1. 
![[Pasted image 20260703115824.png]]

Una vez accedido a la web de configuracion del gateway, se debe acceder al apartado Network, y ahi seleccionar WiFi. El canal se deja en automatico, pero el modo se cambia a Client, para que el gateway sea un cliente mas de la red en la que se conecta para tener acceso a internet.
![[Pasted image 20260703120920.png|717]]

Selecciono la SSID de la red WiFi, coloco el tipo de encriptacion que tiene y luego la contraseña. El protocolo que se usa es DHCP para que le asigne al Gateway un IP en la red, se usa el servidor DNS de la red, y no se sobreescribe el MTU. Para encontrar la IP del gateway una vez se haya cambiado a modo cliente se puede usar el comand nmap -sn (bloque de red) para mostrar las ip conectadas a esa red, y se puede saber cual es la ip del gateway buscando la MAC del mismo. 
## Plan de canal/sub-banda
En el apartado Channel Plan hay que decirle al gateway que
- utilice la region que se utiliza en argentina para LoRa que es AU915-928, 
- se adecue al estandar del protocolo LoRaWAN
- sea publico para que cualquiera lo pueda ver en la red LoRaWAN que este configurado.
- y lo mas importante, que utilice la sub banda (o canales de la frecuencia AU915-928), 2 ya que esta es la que utiliza TTN, que es la red LoRaWAN que utilizaremos con el gateway. 
![[Pasted image 20260703121215.png]]

## TTN
Primero hay que tener o crearse una cuenta para la red [TTN](https://www.thethingsnetwork.org) , teniendo una cuenta
![[Pasted image 20260703124411.png]]
Accedo a la consola
![[Pasted image 20260703124453.png]]
Elijo Norte America 1 ya que es el mas cercano a Argentina. Una vez dentro de la consola en alguno de los clusters, me dirijo a la parte de gateways
![[Pasted image 20260703124707.png]]
Aqui le doy al boton de Register Gateway, para poder registrar el gateway que tengo en la red de TTN. Luego completo los datos de EUI (dato que se encuentra en la etiqueta del dispositivo o en la web de configuracion), id dentro de la red (en este caso ), nombre dentro de la red, y plan de frecuencia, muy importante que este ultimo sea el seleccionado Australia 915-928 FSB 2, ya que esta es la misma frecuencia y sub banda que configuramos en internamente en el gateway.
![[Pasted image 20260703125116.png]]

Finalmente nos dirijimos a la seccion, dentro de nuestro gateway, de API Keys, creamos una, le ponemos un nombre y  los permisos correspondientes, luego de crearla se nos mostrara un valor que hay que guardarlo ya que no se podra volver a acceder al mismo. 

Para poder configurar el gateway con la red TTN tuve que hacer estos pasos EN ORDEN:

1. Resetear de fabrica el gateway, esto es para borrar las posibles conexiones que tenga hechas con anterioridad, y tambien nos permite que se configure como access point para poder acceder a su web de configuracion con la ip por defecto 192.168.230.1. Para llevar a cabo esto se debe presionar el boton de reset durante 5 segundos o mas.

2. Configurar las Network settings de LoRa como Basic Station, ahi cargar los datos
	- URI => Uniform Resorce Identificator, aca ponemos el URI que utiliza la consola donde estemos usando TTN, como estamos en Argentina la mas cercana es la de Norteamerica, por lo que en este apartado colocamos wss://nam1.cloud.thethings.network
	- Port => Es el puerto que utiliza el servidor LNS al que nos estamos conectando, en el caso de TTN es 8887
	- Authentication mode => Para el caso de uso de TTN tenemos que colocar TLS server Authentication and Client Token
		- trust => Aca colocamos un certificado proporcionado por **Let’s Encrypt ISRG ROOT X1 Trust** certificate, se descarga [aqui](https://letsencrypt.org/certs/isrgrootx1.pem)
		- token => Aca copiamos el token generado con la API key desde TTN con el siguiente formato: Authorization: YOUR_API_KEY
3. 
![[Pasted image 20260626140946.png]]
