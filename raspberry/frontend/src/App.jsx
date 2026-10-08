import React, {useEffect, useState} from 'react'

const setpoints = [40,60,80]
const doses = [500,1000,2000]
const faultNames = {NO_SETPOINT:'Llenado deshabilitado',NONE:'Sin alarmas',SENSOR_TIMEOUT:'Sin lectura del sensor',INVALID_LEVEL:'Nivel no válido',MAX_RUN_TIME:'Tiempo máximo de bomba excedido'}
const formatTime = date => date ? new Date(date).toLocaleTimeString('es-ES') : '—'

function Symbol({name}) {
  const icons = {drop:'◉',pump:'↻',network:'⌁',alert:'!',lock:'⌑',clock:'◷'}
  return <span className="symbol" aria-hidden="true">{icons[name] || '•'}</span>
}
function StateDot({ok}) {return <span className={'dot '+(ok?'ok':'error')}/>}
function Metric({label,value,subtitle,icon}) {
  return <article className="metric"><div className="metric-title"><span>{label}</span><Symbol name={icon}/></div><div className="metric-value">{value}</div><div className="metric-subtitle">{subtitle}</div></article>
}
function ControlButton({children,onClick,disabled=false,danger=false,secondary=false}) {
  return <button type="button" onClick={onClick} disabled={disabled} className={'action '+(danger?'danger ':'')+(secondary?'secondary':'')}>{children}</button>
}

export default function App(){
  const [data,setData]=useState(null)
  const [error,setError]=useState('')
  const [lastUpdated,setLastUpdated]=useState(null)
  const [message,setMessage]=useState('')
  const [command,setCommand]=useState(null)
  const [events,setEvents]=useState([])
  const [waterLitres,setWaterLitres]=useState('15')
  const [fertilizerMl,setFertilizerMl]=useState('50')
  const [recipePreview,setRecipePreview]=useState(null)
  const [recipeError,setRecipeError]=useState('')
  const [recipeLoading,setRecipeLoading]=useState(false)
  const [calibrationEnabled,setCalibrationEnabled]=useState(false)
  const [calibrationBusy,setCalibrationBusy]=useState(false)
  const [calibrationLog,setCalibrationLog]=useState([])
  const online=!!data && !error
  useEffect(()=>{
    fetch('/api/controls/capabilities',{cache:'no-store'})
      .then(r=>r.ok?r.json():Promise.reject(Error('no capabilities')))
      .then(result=>setCalibrationEnabled(result.dosing_enabled===true))
      .catch(()=>setCalibrationEnabled(false))
  },[])
  async function dose(action){
    if(calibrationBusy || !calibrationEnabled || !online ) return
    if(action!=='off' && !window.confirm('¿Activar RELAY2 durante '+action+' ms? Verifica que la bomba y los tubos están preparados.')) return
    setCalibrationBusy(true)
    setMessage('')
    try{
      const response=await fetch('/api/dosing/'+action,{
        method:'POST'
      })
      const payload=await response.json()
      if(!response.ok || payload.accepted!==true) throw Error(payload.detail||'Orden rechazada')
      setMessage(action==='off'?'Parada confirmada por el Opta':'Pulso de '+action+' ms aceptado por el Opta')
      setCalibrationLog(old=>[{action,time:new Date().toLocaleTimeString('es-ES')},...old].slice(0,8))
    }catch(e){setMessage('No se ha confirmado la orden: '+e.message)}
    finally{setCalibrationBusy(false)}
  }
  // This build is a UI preview only. Never send physical control requests.
  const commandsEnabled=false
  useEffect(()=>{
    let alive=true, busy=false
    const poll=async()=>{
      if(busy)return
      busy=true
      try{
        const response=await fetch('/api/status',{cache:'no-store'})
        if(!response.ok)throw new Error('HTTP '+response.status)
        const payload=await response.json()
        if(alive){setData(payload);setError('');setLastUpdated(Date.now())}
      }catch(e){
        if(alive){setData(null);setError('Sin respuesta del Opta ('+e.message+')')}
      }finally{busy=false}
    }
    poll()
    const id=setInterval(poll,2000)
    return ()=>{alive=false;clearInterval(id)}
  },[])
  const percent=typeof data?.level_percent==='number'?Math.max(0,Math.min(100,data.level_percent)):null
  const litres=typeof data?.litres==='number'?data.litres.toFixed(2):null
  const fault=data?.fault||'SIN_DATOS'
  const activeAlarm=online&&fault!=='NONE'&&fault!=='NO_SETPOINT'
  function requestCommand(type,value){
    if(!commandsEnabled){setMessage('Controles en preparación: el firmware del Opta aún no acepta órdenes HTTP seguras. No se ha enviado ninguna orden.');return}
    setCommand({type,value})
  }
  async function previewRecipe(event){
    event.preventDefault()
    setRecipeError('')
    setRecipePreview(null)
    setRecipeLoading(true)
    try{
      const result=await fetch('/api/dissolutions/preview',{
        method:'POST',headers:{'Content-Type':'application/json'},
        body:JSON.stringify({water_litres:Number(waterLitres),fertilizer_ml:Number(fertilizerMl)})
      })
      const json=await result.json()
      if(!result.ok) throw new Error(typeof json.detail==='string'?json.detail:'Revisa las cantidades introducidas')
      setRecipePreview(json)
    }catch(e){setRecipeError(e.message)}
    finally{setRecipeLoading(false)}
  }
  return <main className="shell">
    <header className="header">
      <div className="brand"><span className="brandmark">AC</span><div><div className="eyebrow">AUTOMATIZACIÓN AGRÍCOLA</div><h1>AutoCam <span>Corralón</span></h1><p>Panel de control del depósito de mezcla</p></div></div>
      <div className="top-right"><span className={'connection '+(online?'connected':'disconnected')}><StateDot ok={online}/>{online?'Opta conectado':'Opta sin conexión'}</span><span className="readonly"><Symbol name="lock"/> MODO SUPERVISIÓN</span></div>
    </header>

    {error&&<div className="banner error-banner" role="alert"><strong>Comunicación interrumpida</strong><span>{error}. No se muestran estados antiguos como actuales.</span></div>}
    {activeAlarm&&<div className="banner alarm-banner" role="alert"><strong>Alarma activa: {fault}</strong><span>{faultNames[fault]||'Revisar monitor de estado del Opta'}</span></div>}
    {message&&<div className="banner info-banner" role="status"><span>{message}</span><button type="button" onClick={()=>setMessage('')} aria-label="Cerrar mensaje">×</button></div>}

    <div className="section-label"><span>ESTADO EN TIEMPO REAL</span><span>Última actualización: {formatTime(lastUpdated)} · refresco cada 2 s</span></div>
    <div className="overview">
      <section className="tank-card">
        <div className="card-heading"><div><div className="eyebrow">DEPÓSITO PRINCIPAL</div><h2>Nivel de llenado</h2></div><span className="tag neutral">ULTRASONIDOS</span></div>
        <div className="tank-content">
          <div className="tank-illustration" aria-label={percent===null?'Nivel no disponible':'Nivel '+percent+'%'}>
            <div className="tank-top"/>
            <div className="tank-body"><div className="tank-liquid" style={{height:(percent??0)+'%'}}/></div>
            <div className="tank-bottom"/>
          </div>
          <div className="tank-stat"><span className="huge">{percent===null?'—':percent}<small>{percent===null?'':'%'}</small></span><span className="tank-detail">{litres===null?'Lectura no disponible':litres+' litros medidos'}</span><div className="thin-bar"><span style={{width:(percent??0)+'%'}}/></div><div className="tank-caption">Sensor ESP32 → RS485 → Opta</div></div>
        </div>
      </section>
      <div className="metrics">
        <Metric icon="drop" label="Consigna de llenado" value={data?.setpoint_percent==null?'—':data.setpoint_percent+' %'} subtitle="Objetivo configurado en el PLC"/>
        <Metric icon="pump" label="Bomba principal · RELAY1" value={online?String(data.pump_main).toUpperCase():'—'} subtitle="Control automático del llenado"/>
        <Metric icon="pump" label="Peristáltica · RELAY2" value={online?String(data.pump_dosing_1).toUpperCase():'—'} subtitle="Dosificación temporizada"/>
        <Metric icon="network" label="Comunicación" value={online?'ONLINE':'OFFLINE'} subtitle="Opta 192.168.50.2:8080"/>
      </div>
    </div>

    <div className="section-label"><span>ACCIONAMIENTOS</span><span className="label-note"><Symbol name="lock"/> Bloqueados hasta validar control HTTP en el Opta</span></div>
    <div className="control-grid">
      <section className="control-card"><div className="control-head"><span className="control-icon">01</span><div><h2>Llenado del depósito</h2><p>RELAY1 · Control automático mediante consigna</p></div></div>
        <div className="control-description">Seleccionar la consigna deseada. El Opta decide cuándo accionar la bomba aplicando sus protecciones.</div>
        <div className="button-grid">{setpoints.map(n=><ControlButton key={n} disabled={!online||!commandsEnabled} onClick={()=>requestCommand('setpoint',n)}>{n} %</ControlButton>)}</div>
        <div className="button-grid two"><ControlButton danger disabled={!online||!commandsEnabled} onClick={()=>requestCommand('stop')}>Detener llenado</ControlButton><ControlButton secondary disabled={!online||!commandsEnabled} onClick={()=>requestCommand('reset')}>Rearmar alarma</ControlButton></div>
      </section>
      <section className="control-card"><div className="control-head"><span className="control-icon">02</span><div><h2>Bomba peristáltica</h2><p>RELAY2 · Dosificación manual temporizada</p></div></div>
        <div className="control-description">Impulsos limitados por duración. La bomba debe detenerse automáticamente en el Opta.</div>
        <p className="control-description">{calibrationEnabled?'Control de calibración disponible en red local; comprobar que el Opta admite la API de dosificación.':'Dosificación deshabilitada en la Raspberry hasta terminar las pruebas de los relés.'}</p>
        <div className="button-grid">{doses.map(ms=><ControlButton key={ms} disabled={!online||!calibrationEnabled||calibrationBusy} onClick={()=>dose(ms)}>{ms} ms</ControlButton>)}</div>
        <div className="button-grid one"><ControlButton danger disabled={!online||!calibrationEnabled||calibrationBusy} onClick={()=>dose('off')}>Parar peristáltica</ControlButton></div>
        {calibrationLog.length>0&&<div className="calibration-log"><strong>Pulsos confirmados</strong>{calibrationLog.map((item,i)=><p key={i}>{item.time} · {item.action==='off'?'Parada':item.action+' ms'}</p>)}</div>}
      </section>
    </div>
    <div className="section-label"><span>PREPARACIÓN DE DISOLUCIONES</span><span>Planificación sin accionamiento físico</span></div>
    <section className="control-card recipe">
      <div className="control-head"><span className="control-icon">03</span><div><h2>Nueva disolución</h2><p>Define el agua objetivo y los mililitros de fertilizante</p></div></div>
      <form onSubmit={previewRecipe} className="recipe-form">
        <label>Agua objetivo en el depósito (litros)
          <input type="number" min="0.1" max="20" step="0.1" required value={waterLitres} onChange={e=>{setWaterLitres(e.target.value);setRecipePreview(null)}}/>
        </label>
        <label>Fertilizante a dosificar (ml)
          <input type="number" min="0.1" max="120" step="0.1" required value={fertilizerMl} onChange={e=>{setFertilizerMl(e.target.value);setRecipePreview(null)}}/>
        </label>
        <button className="action" type="submit" disabled={!online||recipeLoading}>{recipeLoading?'Validando...':'Calcular receta'}</button>
      </form>
      <p className="recipe-note">Los litros indicados son el <strong>volumen de agua objetivo</strong>, no litros adicionales. El depósito debe comenzar vacío (lectura ≤ 0,10 L, tolerancia provisional). Vaciado manual obligatorio entre recetas. Capacidad de referencia: 20 L.</p>
      {recipeError&&<p role="alert" className="banner error-banner">{recipeError}</p>}
      {recipePreview&&<div className="recipe-result" role="status">
        <h3>Plan de preparación (sin ejecutar)</h3>
        <p>Depósito vacío confirmado para la simulación · Agua solicitada: <strong>{recipePreview.water_target_litres} L</strong></p>
        <p>Dosificación con agua: <strong>{recipePreview.fertilizer_ml} ml</strong> · Tiempo estimado RELAY2: <strong>{recipePreview.estimated_dosing_seconds} s</strong> · Volumen final aproximado: <strong>{recipePreview.estimated_final_volume_litres} L</strong></p>
        <ol><li>Verificar depósito vacío</li><li>Llenado con RELAY1</li><li>Comprobar estabilidad y RELAY1 apagado</li><li>Dosificación con RELAY2</li></ol>
        <button className="action" disabled title="La máquina de estados aún no está integrada ni validada sobre el firmware real del Opta">Iniciar preparación · pendiente</button>
      </div>}
    </section>
    <div className="bottom-grid">
      <section className="info-card"><div className="eyebrow">ESTADO DE PROTECCIONES</div><h2>{online?(faultNames[fault]||fault):'Sin comunicación'}</h2><p>Código comunicado por el Opta: <code>{online?fault:'—'}</code></p></section>
      <section className="info-card"><div className="eyebrow">ARQUITECTURA ACTUAL</div><h2>Raspberry → Opta → ESP32</h2><p>Supervisión por HTTP local, nivel por RS485 y salidas gobernadas exclusivamente por el Opta.</p></section>
    </div>
    <footer>AutoCam Corralón · Dashboard v0.3 · Interfaz de control preparada, accionamientos remotos deshabilitados</footer>
  </main>
}
