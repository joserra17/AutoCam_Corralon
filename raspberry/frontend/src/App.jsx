import React, {useEffect, useState} from 'react'
function Info({title,value,detail}){return <section className="card"><small>{title}</small><h2>{value}</h2><p>{detail}</p></section>}
export default function App(){
  const [data,setData]=useState(null); const [error,setError]=useState('')
  useEffect(()=>{
    let live=true
    async function poll(){
      try{
        const res=await fetch('/api/status',{cache:'no-store'})
        if(!res.ok)throw Error('HTTP '+res.status)
        const json=await res.json();if(live){setData(json);setError('')}
      }catch(e){if(live){setData(null);setError(e.message)}}
    }
    poll();const id=setInterval(poll,2000);return()=>{live=false;clearInterval(id)}
  },[])
  return <main><header><div><h1>AutoCam Corralón</h1><p>Supervisión local · Raspberry Pi</p></div><strong className="chip">MODO SOLO LECTURA</strong></header>
    <p className="warning">La conexión con el Opta aún no ha sido verificada. Todas las órdenes están bloqueadas hasta probar comunicaciones y enclavamientos.</p>
    {error&&<p role="alert" className="warning">API no disponible: {error}</p>}
    <div className="cards">
      <Info title="Nivel del depósito" value={data?.level_percent==null?'Sin datos':data.level_percent+' %'} detail={data?.litres==null?'Esperando telemetría':data.litres+' L'}/>
      <Info title="Consigna" value={data?.setpoint_percent==null?'Sin datos':data.setpoint_percent+' %'} detail="Solo lectura"/>
      <Info title="Bomba principal" value={data?.pump_main??'Desconocida'} detail="Salida RELAY1 · estado no confirmado"/>
      <Info title="Comunicación Opta" value={data?.opta??'Sin conexión'} detail="Modbus TCP pendiente"/>
    </div>
    <h2>Actuadores</h2><div className="cards">{['Peristáltica A','Peristáltica B','Peristáltica C','Riego 1','Riego 2','Riego 3','Riego 4'].map((name,i)=><section className="card" key={name}><strong>{name}</strong><p>Estado: {i<3?(data?.pump_dosing?.[i]??'desconocido'):(data?.valves?.[i-3]??'desconocido')}</p><button disabled title="Pendiente de validar el protocolo con el Opta">Control deshabilitado</button></section>)}</div>
    <footer>API: {data?.mode??'no disponible'} · No hay órdenes físicas habilitadas</footer>
  </main>
}
