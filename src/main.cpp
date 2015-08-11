#include "main.h"
#include <ctime>

#include "CS148/Scene.h"
#include "CS148/PerlinNoise.h"

GLuint texName;

using namespace std;

//****************************************************
// Global Variables
//****************************************************
Viewport * viewport;
mat4 viewToWorld;
World* worlds[MAX_TIME];
Film * film;
Scene * scene;

float rayNum = 0;
float last_percent = 0;
//-------------------------------------------------------------------------------
bool refract(const vec3& d, const vec3& n, double refractive_index, vec3& t) {
    float cosa = -(d * n);
    double cn = refractive_index;
    vec3 normal = n;
    if (cosa < 0) {
        cosa *= -1;
        normal *= -1;
        //cn = 1 / cn; // Handled in calling function.
    }
    float disc = 1 - (1 - cosa * cosa) / (cn * cn);
    if (disc < 0) return false;
    t = d / cn + normal * cosa / cn - normal * sqrt(disc);
    return true;
}

RGB reflectionColor(World* world, Ray& view_ray, vec3& point, const vec3& normal, int depth) {
    if (depth > 0) {
        vec3 view_direction = -1 * vec3(view_ray.direction(), VW);
        view_direction.normalize();
        vec3 bounce_direction = 2 * (normal * view_direction) * normal - view_direction;
        vec3 end = point + bounce_direction;
        Ray bounceRay(point, end, 0.1);
        return traceRay(bounceRay, world, depth - 1);
    }
    return RGB(0,0,0);
}

bool refractionColor(World* world, vec3& direction, double refraction_ratio, vec3& point, const vec3& normal, int depth, vec3& refraction_direction, RGB& refraction_color) {
    refraction_color = RGB(0,0,0);
    if (depth > 0) {
        if (refract(direction, normal, refraction_ratio, refraction_direction)) {
            vec3 start = point;
            vec3 end = point + refraction_direction;
            Ray refracting_ray(start, end, 0.1);
            refraction_color = traceRay(refracting_ray, world, depth  - 1);
            return true;
        }
    }
    return false;
}

RGB combineReflectionRefraction(World* world, Ray& view_ray, Primitive& intersecting, vec3& point, const vec3& normal, int depth) {
    if (depth > 0 && intersecting.getMaterial().getMT() > 0) {
        vec3 direction = vec3(view_ray.direction(), VW);
        direction.normalize();
        double refraction_ratio = 1 / intersecting.getMaterial().getMTN();
        vec3 reflection_normal = normal;
        if (-direction * normal < 0) {
            reflection_normal *= -1;
            refraction_ratio = 1 / refraction_ratio;
        }
        
        RGB color = reflectionColor(world, view_ray, point, reflection_normal, depth);
        
        if (USE_REFRACTION) {
            vec3 refractDirection;
            RGB refractColor;
            if (refractionColor(world, direction, refraction_ratio, point, normal, depth, refractDirection, refractColor)) {
                refractDirection.normalize();
                
                double c = (direction * normal < 0) ? -direction * normal : refractDirection * normal;
                double r0 = pow((refraction_ratio - 1) / (refraction_ratio + 1), 2);
                double r = r0 + (1-r0)*pow(1-c, 5);

                color = r * color + (1-r) * refractColor;
            }
        }
        return color;
    }
    return RGB(0,0,0);
}

double ComputeIlluminationRatio(Light* light, vec4& point, World* world) {
    bool use_dist;
    Ray light_ray = light->getShadowRay(point, use_dist);
    if (PointLight* point_light = dynamic_cast<PointLight*>(light)) {
        vector<pair<Ray, double>> shadow_rays = point_light->GetShadowRays(point, use_dist);
        double illumination_ratio = 0.0;
        for (auto& ray_tmax : shadow_rays) {
            double t_s;
            if (world->intersect(ray_tmax.first, t_s) == nullptr || t_s >= ray_tmax.second) {
                illumination_ratio += 1;
            }
        }
        return illumination_ratio / shadow_rays.size();
    }
    
    double t_l;
    return world->intersect(light_ray, t_l) == nullptr || t_l >= std::numeric_limits<float>::infinity() ? 1.0 : 0.0;
}

// Here you raycast a single ray, calculating the color of this ray.
RGB traceRay(Ray & ray, World* world, int depth) {
    RGB retColor(0,0,0);
    double t;
    Primitive* intersecting = world->intersect(ray, t);
    float percent_done = rayNum / (IMAGE_HEIGHT*IMAGE_WIDTH*RAYS_PER_PIXEL) * 100;
    if (percent_done - last_percent > 1) {
        last_percent = percent_done;
        cout << (int)percent_done << " % done" << endl;
    }
    if (intersecting != nullptr) {
        vec4 point = ray.getPos(t);
        vec3 p(point);
        //return intersecting->getColor(p);
        vec3 n(intersecting->calculateNormal(point));
        vector<Light*>::const_iterator light_it = world->getLightsBeginIterator();
        while(light_it != world->getLightsEndIterator()) {
            RGB Spec = (intersecting->getMaterial().getMSM()) * intersecting->getColor(p) +
            (1 - intersecting->getMaterial().getMSM()) * RGB(1, 1, 1);
            Light* light = *light_it;
            vec3 l_dir; light->getIncidenceVector(point, l_dir);

            double illumination_ratio = ComputeIlluminationRatio(light, point, world);
            if (illumination_ratio > 0.0) {
                // This point is not in shadow - lets compute the phong color
                vec3 d = viewport->getViewVector(p);
                vec3 r = 2 * (n * l_dir) * n - l_dir;
                RGB LambComp = intersecting->getMaterial().getML() * intersecting->getColor(p) * light->getColor(point) * MAX(l_dir*n, 0);
                RGB SpecComp = intersecting->getMaterial().getMS() * Spec * light->getColor(point) *
                pow(MAX(0, r*d), intersecting->getMaterial().getMSP());
                retColor += illumination_ratio * (LambComp + SpecComp);
            }
            ++light_it;
        }
        if (depth > 0 && intersecting->getMaterial().getMT() > 0.0) {
            double k = intersecting->getMaterial().getMT();
            vec3 direction = vec3(ray.direction(), VW);
            direction.normalize();
            if (-direction * n < 0) {
                k = 1.0;
            }
            retColor *= (1 - k);
            retColor += k * combineReflectionRefraction(world, ray, *intersecting, p, n, depth);
        }
        RGB AmbComp = intersecting->getMaterial().getMA() * intersecting->getColor(p) * world->getAmbientLightColor();
        retColor += AmbComp;

    }
    retColor.clip(1);
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
        for (Ray ray : viewport->createViewingRays(sample)) {
            c = RGB(0,0,0);
            ray.transform(viewToWorld);  //transform this to world space
            // For every ray - choose a random world (for motion blur)
            int rand_num = rand()%MAX_TIME;
            World* world = worlds[rand_num];
            c += traceRay(ray, world, MAX_BOUNCES);
            ++rayNum;
            film->expose(c, sample);
        }
    }
	film->bakeAndSave();
	cout << "Image saved!" << endl;
}

//-------------------------------------------------------------------------------
// This traverses the read-in scene file's DAG and builds a list of
// primitives, lights and the viewport object. See World.h
void sceneToWorld(SceneInstance *inst, World* world, mat4 localToWorld, int time) {
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
        sceneToWorld(g->getChild(i), world, localToWorld, time);
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

        viewport = new Viewport(eye, LL, UL, LR, UR, IMAGE_WIDTH, IMAGE_HEIGHT, localToWorld);
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

    double r, h;
    MaterialInfo m;
    if (g->computeSphere(r, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        Sphere *sph = new Sphere(r, m.color, mat, m.texture, m.bumpTexture, m.noise, localToWorld);
        world->addPrimitive(sph);
    }

    if (g->computeCylinder(r, h, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        Cylinder *cy = new Cylinder(h, r, m.color, mat, localToWorld);
        world->addPrimitive(cy);
    }

    if (g->computeCone(r, h, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        Cone *cone = new Cone(h, r, m.color, mat, localToWorld);
        world->addPrimitive(cone);
    }

    VertexInfo vert;
    if (g->computeTriangle(vert, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        Triangle *tri = new Triangle(vec3(vert.vertices[0], vert.vertices[1], 0),
                                     vec3(vert.vertices[2], vert.vertices[3], 0),
                                     vec3(vert.vertices[4], vert.vertices[5], 0),
                                     m.color, mat, m.texture, m.bumpTexture, localToWorld);
        world->addPrimitive(tri);
    }

    if (g->computeCuboid(vert, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        Cuboid *cuboid = new Cuboid(vec3(vert.vertices[0], vert.vertices[1], vert.vertices[2]), vec3(vert.vertices[3], vert.vertices[4], vert.vertices[5]), m.color, mat, localToWorld);
        world->addPrimitive(cuboid);
    }
    
    OBJTriangleMesh *t;
    if (g->computeMesh(t, m, time)) {
        Material mat(m.k[0],m.k[1],m.k[2],m.k[3],m.k[4],m.k[MAT_MS],m.k[5],m.k[6]);
        int num = 0;
        for (vector<OBJTriangle*>::iterator it = t->triangles.begin(); it != t->triangles.end(); ++it) {
            ++num;
            Triangle* tri;
            if (t->verticesHaveTexture() && t->verticesHaveNormal()) {
               tri = new Triangle(t->vertices[ (*it)->ind[0] ]->pos,
                                  t->textures[ (*it)->texture_ind[0] ]->coords,
                                  t->normals[ (*it)->normal_ind[0] ]->dir,
                                  t->vertices[ (*it)->ind[1] ]->pos,
                                  t->textures[ (*it)->texture_ind[1] ]->coords,
                                  t->normals[ (*it)->normal_ind[1] ]->dir,
                                  t->vertices[ (*it)->ind[2] ]->pos,
                                  t->textures[ (*it)->texture_ind[2] ]->coords,
                                  t->normals[ (*it)->normal_ind[2] ]->dir,
                                  m.color, mat, m.texture, m.bumpTexture, localToWorld);
            } else if (t->verticesHaveNormal()) {
                tri = new Triangle(t->vertices[ (*it)->ind[0] ]->pos,
                                   t->normals[ (*it)->normal_ind[0] ]->dir,
                                   t->vertices[ (*it)->ind[1] ]->pos,
                                   t->normals[ (*it)->normal_ind[1] ]->dir,
                                   t->vertices[ (*it)->ind[2] ]->pos,
                                   t->normals[ (*it)->normal_ind[2] ]->dir,
                                   m.color, mat, m.texture, m.bumpTexture, localToWorld);
            } else if (t->verticesHaveTexture()) {
                tri = new Triangle(t->vertices[ (*it)->ind[0] ]->pos,
                                   t->textures[ (*it)->texture_ind[0] ]->coords,
                                   t->vertices[ (*it)->ind[1] ]->pos,
                                   t->textures[ (*it)->texture_ind[1] ]->coords,
                                   t->vertices[ (*it)->ind[2] ]->pos,
                                   t->textures[ (*it)->texture_ind[2] ]->coords,
                                   m.color, mat, m.texture, m.bumpTexture, localToWorld);
            } else {
                tri = new Triangle(t->vertices[ (*it)->ind[0] ]->pos,
                                   t->vertices[ (*it)->ind[1] ]->pos,
                                   t->vertices[ (*it)->ind[2] ]->pos,
                                   m.color, mat, m.texture, m.bumpTexture, localToWorld);
            }
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

void GenerateHeightMap() {
    PerlinNoise noise(0.1, 10, 5, 1, rand());
    FreeImage_Initialise();
    FIBITMAP* _output = FreeImage_Allocate(512, 512, 24);  // Generate 512x512 height map image in grayscale
    for (int x = 0; x < 512; ++x) {
        for (int y = 0; y < 512; ++y) {
            RGBQUAD color;
            float height = noise.GetHeight(float(x)/512, float(y)/512);
            color.rgbRed = height * 255;
            color.rgbGreen = height * 255;
            color.rgbBlue = height * 255;
            FreeImage_SetPixelColor(_output, x, y, &color);
        }
        FreeImage_Save(FIF_PNG, _output, "heightmap.png");
    }
}

/// Initialize the environment
int main(int argc,char** argv) {
    //srand (time(NULL));
    const clock_t begin_time = clock();
    
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
    for (int time = 0; time < MAX_TIME; ++time) {
        World* world = new World();
        sceneToWorld(scene->getRoot(), world, identity3D(), time);
        if (USE_ACCELERATION_INDEX) {
            world->PreprocessToBHVTree();
        }
        if (USE_VERTEX_NORMALS) {
            world->PreprocessTriangleNormals();
        }
        worlds[time] = world;
        if (time == MAX_TIME - 1) world->printStats();
    }
    cout << "Imported Scene File." << endl;
    renderWithRaycasting();
    cout << "Time taken : " << float(clock() - begin_time)/CLOCKS_PER_SEC << endl;
}


