(Mtllib "tests/city/city.mtl")
(MatMesh "tests/city/city.obj")

#(Material mat1
#    (color .5 .9 .4)
#    (ma .5) (ml .5) (ms .3) (msp 4) (msm .5) (mt 0.5) (mtn 1)
#)

#(Include mesh "tests/city/city.obj" (material mat1))

(Light amb
    (type 0) (color .5 .2 .2)
)

(Light dir
    (type 1) (color 1 1 1)
)

(Camera cam
    (perspective 1)
    (l -1) (r 1) (b -1) (t 1) (n 3) # no far plane 
)

(G threespheres
    (I cam1 cam)
    (I dir1 dir (Xform 0 0 -.5 0
                       0 0 .5 0
                       0 0 .5 0
                       0 0 0 0)) # points toward (.5,.5,-.5)
    (I amb1 amb)
    (I b1 mesh01 (T 0 -500 -1000))
    (I b2 bldgs2 (T 0 -500 -1000))
    (I b3 sidewalks (T 0 -500 -1000))
    (I b4 def (T 0 -500 -1000))
#    (I city mesh)
)

(Render  threespheres)


