#include "main.h"

#include "CS148/Scene.h"

GLuint texName;

using namespace std;

//****************************************************
// Global Variables
//****************************************************
Viewport * viewport;
mat4 viewToWorld;
World * world;
Film * film;
Scene * scene;


//-------------------------------------------------------------------------------
// Here you raycast a single ray, calculating the color of this ray.
RGB traceRay(Ray & ray, int depth) {
    RGB retColor(0,0,0);
    //YOUR CODE HERE!!!

	//Use the "world" global variable to access the primitives and lights in the input file.
	// write methods on your primitives and lights to help you.


    //Please be sure to read the following classes in algebra3.h:
    // - Color
    // - Point (useful for storing sample information)
    // - Material
    // - Ray


	//IMPORTANT:
	//Please start all bounce rays at a t value of 0.1 - this has the effect of slightly offsetting
	//bounce rays from the surface they're bouncing from, and prevents bounce rays from being occluded by their own surface.

    
    double t;
    Primitive* intersecting = world->intersect(ray, t);
    if (intersecting != nullptr) {
        RGB AmbComp = intersecting->getMaterial().getMA() * intersecting->getColor() * world->getAmbientLightColor();
        vec4 point = ray.getPos(t);
        vec3 n(intersecting->calculateNormal(point));
        vector<Light*>::const_iterator light_it = world->getLightsBeginIterator();
        while(light_it != world->getLightsEndIterator()) {
            RGB Spec = (intersecting->getMaterial().getMSM()) * intersecting->getColor() +
            (1 - intersecting->getMaterial().getMSM()) * RGB(1, 1, 1);

            Light* light = *light_it;
            vec4 l; light->getIncidenceVector(point, l);
            vec3 l_dir(l); l_dir.normalize();
            vec3 p;
            double t_l;
            Ray light_ray(p, l_dir, 0.1);
            if (world->intersect(light_ray, t_l) == nullptr) {
                // This point is not in shadow - lets compute the phong color
                vec3 d(viewport->getViewVector(point));
                RGB LambComp = intersecting->getMaterial().getML() * intersecting->getColor() * light->getColor(point) * MAX(d*n, 0);
                vec3 h = (l_dir + d).normalize();
                RGB SpecComp = intersecting->getMaterial().getMS() * Spec * light->getColor(point) *
                pow(MAX(0, h*n), intersecting->getMaterial().getMSP());
                cout << "!" << SpecComp << endl;
                retColor += LambComp; //  + SpecComp;
            }
            ++light_it;
        }
        retColor += AmbComp;
    }
    return retColor;
}

//-------------------------------------------------------------------------------
// Here you write your main render function. This calls the raycast method
// on each ray, which you generate using the Sampler and Ray classes.
void renderWithRaycasting() {
    Sample sample;    //This is the point on the viewport being sampled.
    Ray ray;        //This is the ray being traced from the eye through the point.
    RGB c(0,0,0);

    viewport->resetSampler();
    while(viewport->getSample(sample)) {  //just gets a 2d sample position on the film
        c = RGB(0,0,0);
    	ray = viewport->createViewingRay(sample);  //convert the 2d sample position to a 3d ray
        ray.transform(viewToWorld);  //transform this to world space
        c += traceRay(ray, 0);
        film->expose(c, sample);
    }
	film->bakeAndSave();
	cout << "Image saved!" << endl;
}

//-------------------------------------------------------------------------------
// This traverses the read-in scene file's DAG and builds a list of
// primitives, lights and the viewport object. See World.h
void sceneToWorld(SceneInstance *inst, mat4 localToWorld, int time) {
    if (inst == NULL)
        return;

    mat4 nodeXform;
    inst->computeTransform(nodeXform,time);
    localToWorld = localToWorld * nodeXform;

    SceneGroup *g = inst->getChild();
    if (g == NULL) { // for example if the whole scene fails to load
        cout << "ERROR: We arrived at an instance with no child?!" << endl;
        return;
    }

    int ccount = g->getChildCount();
    for (int i = 0; i < ccount; i++) {
        sceneToWorld(g->getChild(i), localToWorld, 0);
    }

    CameraInfo f;
    if (g->computeCamera(f, time)) {
        viewToWorld = localToWorld;

        if (viewport != NULL)
            delete viewport;

        vec4 eye(0.0, 0.0, 0.0, 1.0);
        vec4 LL(f.sides[FRUS_LEFT], f.sides[FRUS_BOTTOM], -f.sides[FRUS_NEAR], 1.0);
        vec4 UL(f.sides[FRUS_LEFT], f.sides[FRUS_TOP], -f.sides[FRUS_NEAR], 1.0);
        vec4 LR(f.sides[FRUS_RIGHT], f.sides[FRUS_BOTTOM], -f.sides[FRUS_NEAR], 1.0);
        vec4 UR(f.sides[FRUS_RIGHT], f.sides[FRUS_TOP], -f.sides[FRUS_NEAR], 1.0);

        viewport = new Viewport(eye, LL, UL, LR, UR, IMAGE_WIDTH, IMAGE_HEIGHT);
    }

    LightInfo l;
    if (g->computeLight(l, time)) {
        if (l.type == LIGHT_AMBIENT) {
            RGB amb = world->getAmbientLightColor();
            world->setAmbientLightColor(amb + l.color);
        }
        else if (l.type == LIGHT_DIRECTIONAL) {
            DirectionalLight *li = new DirectionalLight(l.color);
            vec4 dir(0,0,-1,0);
            li->setDirection(localToWorld*dir);
            world->addLight(li);
        }
        else if (l.type == LIGHT_POINT) {
            PointLight *li = new PointLight(l.color, l.falloff, l.deadDistance);
            vec4 pos(0,0,0,1);
            li->setPosition(localToWorld*pos);
            world->addLight(li);
        }
        else if (l.type == LIGHT_SPOT) {
            throw "oh no";
        }
    }

    double r;
    MaterialInfo m;
    if (g->computeSphere(r, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        Sphere *sph = new Sphere(r, m.color, mat, localToWorld);
        world->addPrimitive(sph);
    }

    OBJTriangleMesh *t;
    if (g->computeMesh(t, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        for (vector<OBJTriangle*>::iterator it = t->triangles.begin(); it != t->triangles.end(); ++it) {
            Triangle *tri = new Triangle(
                                t->vertices[ (**it).ind[0] ]->pos,
                                t->vertices[ (**it).ind[1] ]->pos,
                                t->vertices[ (**it).ind[2] ]->pos,
                                m.color, mat, localToWorld);
            world->addPrimitive(tri);
        }
    }

}

//-------------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'q':  //  Quit program.
            exit(0);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 0, 0);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, texName);
    glMatrixMode(GL_MODELVIEW);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 1);
    glVertex3f(-1, 1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-1, -1, 0);
    glTexCoord2f(1, 0);
    glVertex3f(1, -1, 0);
    glColor3f(0, 0, 1.0);
    glTexCoord2f(1, 0);
    glVertex3f(1, -1, 0);
    glTexCoord2f(1, 1);
    glVertex3f(1, 1, 0);
    glTexCoord2f(0, 1);
    glVertex3f(-1, 1, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();
}

void InitOpenGL(int argc,char** argv, Film* film) {
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (IMAGE_WIDTH, IMAGE_HEIGHT);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("RayTracer");
    // SetUp
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glOrtho(-1, 1, -1, 1, -1, 1);
    glEnable(GL_DEPTH_TEST);

    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    film->BindTexture();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    //glutMainLoop();
    glutMainLoopEvent();
}

/// Initialize the environment
int main(int argc,char** argv) {
    if (argc != 3) {
        cout << "USAGE: raytracer scene.scd output.png" << endl;
        exit(1);
    }
    film = new Film(IMAGE_WIDTH, IMAGE_HEIGHT, argv[2]);
    if (OPENGL_RENDER) {
        InitOpenGL(argc, argv, film);
    }
    scene = new Scene(argv[1]);
    viewToWorld = identity3D();
    world = new World();
    sceneToWorld(scene->getRoot(), identity3D(), 0);
    cout << "Imported Scene File." << endl;
    world->printStats();
    renderWithRaycasting();
}


