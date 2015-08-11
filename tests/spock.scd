(Material mesh1mat
    (color .7 .7 .7)
    (ma .1) (ml .3) (ms .7) (msp 100) (msm 1) (mt 0) (mtn 1)
)

(Include mesh "tests/spock.obj" (material mesh1mat))

(Light amb
    (type 0) (color .5 .2 .2)
)

(Light dir
    (type 1) (color .4 .8 1.2)
)

(Light pt
    (type 2) (color 1.39 .2 .2)
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
    (I tea1 mesh (R 90 0 1 0) (R 90 0 0 1) (T 0 0 -4))
    (I pt1 pt (T 2 2 -10))
)

(Render  threespheres)


