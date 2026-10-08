# AutoCam Raspberry — panel local (v1)

La primera versión es **solo lectura / simulación**. No se han implementado órdenes físicas ni Modbus TCP y los estados no representan al Opta real.

## Conexión Ethernet física

Raspberry `eth0`: en una configuración anterior se utilizaba `192.168.50.1/24`; **verificar en el equipo** antes de usar esa dirección. El Opta necesita IP distinta en la misma subred, máscara compatible y un servidor Modbus TCP real para intercambiar datos.

En la Raspberry:
```sh
ip -4 addr show eth0
ip link show eth0
ip neigh show dev eth0
```
Si conoces la IP del Opta (sustituye OPTA_IP):
```sh
ping -c 3 OPTA_IP
```
Si el ping no responde, no demuestra por sí solo que esté apagado: revisar configuración IP, enlace y servicios. Si se espera Modbus TCP:
```sh
nc -vz -w 2 OPTA_IP 502
```
Puerto 502 abierto no demuestra todavía que exista un contrato de registros funcional.

## Backend
```sh
cd raspberry/backend
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
uvicorn app.main:app --host 0.0.0.0 --port 8000
```
Comprobar `http://127.0.0.1:8000/api/health` en Raspberry.

## Frontend
Abrir otra terminal:
```sh
cd raspberry/frontend
npm install
npm run dev
```
Abrir `http://IP_DE_RASPBERRY:5173` desde otro equipo de la red, si la red lo permite.

**Seguridad:** esto es un servidor de desarrollo. No exponer a Internet; antes de pasar a producción se implementarán autenticación, servicio persistente y proxy inverso.

## Siguientes entregas
1. Confirmar IP y acceso TCP al Opta.
2. Implementar contrato de registros Modbus TCP en Opta y documentarlo en `docs/modbus-map.md`.
3. Añadir cliente Modbus al backend, lecturas verificadas, watchdog y control con autorización, enclavamientos y confirmaciones.
4. Implementar programaciones y persistencia.
