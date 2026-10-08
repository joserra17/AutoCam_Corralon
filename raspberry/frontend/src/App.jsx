import React, {useEffect, useState} from 'react'

function Info({title,value,detail}) {
  return <section className="card"><small>{title}</small><h2>{value}</h2><p>{detail}</p></section>
}

export default function App() {
  const [data,setData] = useState(null)
  const [error,setError] = useState('')
  useEffect(() => {
    let active = true
    let inFlight = false
    async function poll() {
      if (inFlight) return
      inFlight = true
      try {
        const response = await fetch('/api/status', {cache:'no-store'})
        if (!response.ok) throw new Error('HTTP '+response.status)
        const result = await response.json()
        if (active) { setData(result); setError('') }
      } catch (e) {
        if (active) { setData(null); setError('Sin telemetría del Opta ('+e.message+')') }
      } finally { inFlight = false }
    }
    poll()
    const timer = setInterval(poll, 2000)
    return () => { active = false; clearInterval(timer) }
  }, [])

  const online = Boolean(data && !error)
  const level = typeof data?.level_percent === 'number' ? data.level_percent : null
  const volume = typeof data?.litres === 'number' ? data.litres.toFixed(2)+' L' : 'Sin lectura reciente'
  return <main>
    <header>
      <div><h1>AutoCam Corralón</h1><p>Supervisión local · Raspberry Pi ↔ Opta Ethernet</p></div>
      <strong className="chip">SOLO LECTURA</strong>
    </header>
    <p className={online?'notice':'warning'} role="status">
      {online?'Opta conectado · datos obtenidos de la API HTTP':'Opta sin comunicación · salidas físicas no controlables desde este panel'}
    </p>
    {error && <p className="warning" role="alert">{error}</p>}
    <div className="cards">
      <Info title="Nivel del depósito" value={level===null?'Sin datos':level+' %'} detail={volume}/>
      <Info title="Consigna" value={data?.setpoint_percent==null?'Sin datos':data.setpoint_percent+' %'} detail="Configurada en el Opta"/>
      <Info title="Bomba principal · RELAY1" value={data?.pump_main?.toUpperCase()??'SIN DATOS'} detail="Estado comunicado por el Opta"/>
      <Info title="Peristáltica · RELAY2" value={data?.pump_dosing_1?.toUpperCase()??'SIN DATOS'} detail="Estado comunicado por el Opta"/>
      <Info title="Alarma / condición" value={data?.fault??'SIN DATOS'} detail="NO_SETPOINT indica llenado deshabilitado"/>
      <Info title="Conexión Opta" value={online?'CONECTADO':'DESCONECTADO'} detail={data?.timestamp?'Última consulta: '+new Date(data.timestamp).toLocaleTimeString():'Esperando respuesta'}/>
    </div>
    {level!==null && <section className="card"><strong>Depósito</strong><div className="levelbar"><div className="levelbar-fill" style={{width:Math.max(0,Math.min(100,level))+'%'}}/></div><p>{level} % · {volume}</p></section>}
    <footer>Consulta automática cada 2 s · HTTP local · Sin comandos de control habilitados</footer>
  </main>
}
