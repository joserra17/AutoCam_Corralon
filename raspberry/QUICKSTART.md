# AutoCam Raspberry — dashboard local conectado al Opta

Esta rama es **solo lectura**. El backend consulta por HTTP la API real del Opta; las órdenes de control están expresamente deshabilitadas.

## Red

- Raspberry eth0: `192.168.50.1/24` (perfil NetworkManager `autocam-opta` en el equipo probado).
- Opta: `192.168.50.2`, puerto TCP `8080`.
- ESP32 envía el nivel por RS485 al Opta. La Raspberry **no necesita** conexión directa con la ESP32.

Comprueba desde la Raspberry:
```bash
ip -4 addr show eth0
curl --max-time 5 -i http://192.168.50.2:8080/api/status
```

## Instalar backend FastAPI
```bash
cd ~/AutoCam_Corralon/raspberry/backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
export OPTA_STATUS_URL=http://192.168.50.2:8080/api/status
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

En otra sesión de Raspberry:
```bash
curl -i http://127.0.0.1:8000/api/health
curl -i http://127.0.0.1:8000/api/status
```

Si el Opta se desconecta, `/api/status` responde HTTP 503 en lugar de inventar valores. El dashboard mostrará «SIN DATOS» y «DESCONECTADO».

## Frontend React / Vite
Necesita Node.js y npm instalados. En otra terminal:
```bash
cd ~/AutoCam_Corralon/raspberry/frontend
npm install
npm run dev -- --host 0.0.0.0
```

Abre `http://IP_RASPBERRY:5173` desde un dispositivo que tenga acceso a esa IP. El frontend consulta `/api/status` mediante el proxy de Vite al puerto 8000 (revisa `vite.config.js` si usas otros puertos).

## Seguridad y alcance
- El dashboard **no** envía SETPOINT ni DOSE al Opta.
- No habilitar un puerto hacia Internet ni redirigir el 8080 públicamente.
- Esta configuración es de desarrollo; faltan HTTPS, autenticación y servicio persistente.
- RELAY1 y RELAY2 dependen de la lógica local del Opta; su seguridad no debe depender de FastAPI.
