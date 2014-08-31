fn setKRKALcamera = (

  c = $KRKALcamera
  if c != undefined then
     delete c

  c = freecamera name:"KRKALcamera" targetdistance:80 fov:37.30 orthoProjection:on


  m = matrix3 [1,0,0] [0,1,0] [.35,-.35,1] [21,-21,80]

  c.transform = m

  freeze c
  hide c

)

fn setKRKALlight shadows = (

  p=$ShadowBase
  if p!=undefined then
  	delete p

  l=$KRKALlight
  if l != undefined then
    delete l


  l = directionallight name:"KRKALlight" castshadows:shadows raytracedshadows:on hotspot:10000 falloff:10002 overshoot:on


  rot = eulerangles 30 30 0
  rotate l rot
  move l [0,0,200]
  
  freeze l
  hide l
  
  if shadows==true then
  (
    g=$geometry
	g.primaryVisibility = off
  	plane name:"ShadowBase" width:100 length:100 Length_Segments:1 Width_Segments:1
  )
  else
  (
    g=$geometry
	g.primaryVisibility = on
  )
)

fn setKRKALrenelems shadows = (
  remgr = maxOps.GetCurRenderElementMgr()
  remgr.removeallrenderelements()

  if shadows == false then
  (

  el=KRKAL_Z_DEPTH
  remgr.addrenderelement (el elementname:(el as string))
  zdepth = remgr.GetRenderElement(0)

  zdepth.zmult = 0.819
  zdepth.z_min=0
  zdepth.z_max=255
  
  el=KRKAL_ALPHA
  remgr.addrenderelement (el elementname:(el as string))
  el=KRKAL_DIFFUSE
  remgr.addrenderelement (el elementname:(el as string))
  el=KRKAL_NORMAL
  remgr.addrenderelement (el elementname:(el as string))
  
  )
  else
  (
    el=KRKAL_SHADOW
	remgr.addrenderelement (el elementname:(el as string))
  )
  
) 

shadows = off

setKRKALcamera () 
setKRKALlight (shadows)
setKRKALrenelems (shadows)