(Material cubemat
    (texture "cs148-cube.png")
    (ma .1) (ml .7) (ms 0) (msp 100) (msm 1) (mt 0) (mtn 1)
)

(Material spheremat
     (texture "worldmap.png")
     (ma .1) (ml .7) (ms 0) (msp 100) (msm 1) (mt 0) (mtn 1)
)

(Include mesh "tex-cube.obj" (material cubemat))
(Sphere sph1 (radius 2) (material spheremat))

(Light amb
    (type 0) (color 1 1 1)
)

(Light dir
    (type 1) (color 1 1 1)
)

(Light pt
    (type 2) (color 1 1 1)
)

(Camera cam
    (perspective 1)
    (l -1) (r 1) (b -1) (t 1) (n 3) # no far plane 
)

(G threespheres
    (I cam1 cam)
    (I dir1 dir (Xform 0 0 -.5 0
                           0 0 -.5 0
                           0 0 .5 0
                           0 0 0 0)) # points toward (.5,.5,-.5)
    (I amb1 amb)
    (I s1 sph1 (T -2.5 -1.5 -17.0))
    (I bx mesh (R 45 0 1 0) (R 30 1 0 0) (T 1 -1 -11))
    (I pt1 pt (T 2 2 -8))
)

(Render  threespheres)


