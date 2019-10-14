/** CSci-4611 Assignment 2:  Car Soccer
 */

 #include "car_soccer.h"
 #include "config.h"
 #include <math.h>


 CarSoccer::CarSoccer() : GraphicsApp(1024,768, "Car Soccer") {
     // Define a search path for finding data files (images and shaders)
     searchPath_.push_back(".");
     searchPath_.push_back("./data");
     searchPath_.push_back(DATA_DIR_INSTALL);
     searchPath_.push_back(DATA_DIR_BUILD);
 }

 CarSoccer::~CarSoccer() {
 }


 Vector2 CarSoccer::joystick_direction() {
     Vector2 dir;
     if (IsKeyDown(GLFW_KEY_LEFT))
         dir[0]--;
     if (IsKeyDown(GLFW_KEY_RIGHT))
         dir[0]++;
     if (IsKeyDown(GLFW_KEY_UP))
         dir[1]++;
     if (IsKeyDown(GLFW_KEY_DOWN))
         dir[1]--;
     return dir;
 }

void CarSoccer::launch_ball(){
  int vx = rand() % 10 + 1;
  int vz = rand() % 15 + 1;
  ball_.set_velocity(Vector3(vx,0,vz));
}

 void CarSoccer::OnSpecialKeyDown(int key, int scancode, int modifiers) {
     if (key == GLFW_KEY_SPACE) {
         // Here's where you could call some form of launch_ball();
         ball_.Reset();
         launch_ball();
         car_.Reset();
     }
 }


 void CarSoccer::UpdateSimulation(double timeStep) {
     // Here's where you shound do your "simulation", updating the positions of the
     // car and ball as needed and checking for collisions.  Filling this routine
     // in is the main part of the assignment.

     ball_.set_position(ball_.position() + ball_.velocity()*timeStep);

     //ball-wall collisions
     //decrease the velocity when bouncing to the wall by multiplying 0.8
     //bounce to the right and left walls
     if(ball_.position()[0]+ball_.radius() > 40){
       Vector3 n(-1,0,0);
       Vector3 r = 0.8*(ball_.velocity() - 2.0*(ball_.velocity().Dot(n))*n);
       ball_.set_velocity(r);
     }
     if(ball_.position()[0]-ball_.radius() < -40){
       Vector3 n(1,0,0);
       Vector3 r = 0.8*(ball_.velocity() - 2.0*(ball_.velocity().Dot(n))*n);
       ball_.set_velocity(r);
     }
     // //bounce to the ground and ceiling
     if(ball_.position()[1]+ball_.radius() > 35){
       Vector3 n(0,-1,0);
       Vector3 r = 0.8*(ball_.velocity() - 2.0*(ball_.velocity().Dot(n))*n);
       ball_.set_velocity(r);
     }
     if(ball_.position()[1] < ball_.radius()){
       Vector3 n(0,1,0);
       Vector3 r = 0.8*(ball_.velocity() - 2.0*(ball_.velocity().Dot(n))*n);
       ball_.set_velocity(r);
     }
     //bounce to the front and back walls
     if(ball_.position()[2]+ball_.radius() > 50){
       //get the goal
       if(ball_.position()[0]>-10 && ball_.position()[0] < 10 && ball_.position()[1] < 10){
         ball_.Reset();
         car_.Reset();
         launch_ball();
       }
       else{
         Vector3 n(0,0,-1);
         Vector3 r = 0.8*(ball_.velocity() - 2.0*(ball_.velocity().Dot(n))*n);
         ball_.set_velocity(r);
       }
     }
     if(ball_.position()[2] - ball_.radius() < -50){
       //get the goal
       if(ball_.position()[0]>-10 && ball_.position()[0] < 10 && ball_.position()[1] < 10){
         ball_.Reset();
         car_.Reset();
         launch_ball();
       }
       else{
         Vector3 n(0,0,1);
         Vector3 r = 0.8*(ball_.velocity() - 2.0*(ball_.velocity().Dot(n))*n);
         ball_.set_velocity(r);
       }
     }

     //Move the car
     Vector2 control = joystick_direction();
     float speed = car_.speed() + control[1]*timeStep;
     float sX = speed * sin(car_.angle());
     float sZ = speed * cos(car_.angle());
     car_.set_position(car_.position() - Vector3(sX, 0, sZ));
     car_.set_angle(car_.angle()+ 2 * control[0] * timeStep);
     car_.set_speed(speed);

     //car-wall collision
     //collide with right and left walls
     Point3 carPos = car_.position();
     if(carPos[0]+car_.collision_radius() > 40){
       carPos[0] = 40 - car_.collision_radius();
     }
     if(carPos[0]-car_.collision_radius() < -40){
       carPos[0] = -40 + car_.collision_radius();
     }
     //collide with front and back walls
     if(carPos[2]+car_.collision_radius() > 50){
       carPos[2] = 50 - car_.collision_radius();
     }
     if(carPos[2]-car_.collision_radius() < -50){
       carPos[2] = -50 + car_.collision_radius();
     }
     car_.set_position(carPos);

     //car-ball collision
     float distance = (car_.position() - ball_.position()).Length();
     if(distance < car_.collision_radius()+ball_.radius()){
       Vector3 ball_dir = ball_.velocity().ToUnit();
       float penetration = ball_.radius() + car_.collision_radius() - distance;
       ball_.set_position(ball_.position()-penetration*ball_dir);
       Vector3 new_v = 0.8*(ball_.velocity() - 2.0*(ball_.velocity().Dot(ball_dir))*ball_dir);
       ball_.set_velocity(new_v);
     }
     ball_.set_position(ball_.position()+ball_.velocity()*timeStep);

 }


 void CarSoccer::InitOpenGL() {
     // Set up the camera in a good position to see the entire field
     projMatrix_ = Matrix4::Perspective(60, aspect_ratio(), 1, 1000);
     modelMatrix_ = Matrix4::LookAt(Point3(0,60,70), Point3(0,0,10), Vector3(0,1,0));

     // Set a background color for the screen
     glClearColor(0.8,0.8,0.8, 1);

     // Load some image files we'll use
     fieldTex_.InitFromFile(Platform::FindFile("pitch.png", searchPath_));
     crowdTex_.InitFromFile(Platform::FindFile("crowd.png", searchPath_));
 }


 void CarSoccer::DrawUsingOpenGL() {
     // Draw the crowd as a fullscreen background image
     quickShapes_.DrawFullscreenTexture(Color(1,1,1), crowdTex_);

     // Draw the field with the field texture on it.
     Color col(16.0/255.0, 46.0/255.0, 9.0/255.0);
     Matrix4 M = Matrix4::Translation(Vector3(0,-0.201,0)) * Matrix4::Scale(Vector3(50, 1, 60));
     quickShapes_.DrawSquare(modelMatrix_ * M, viewMatrix_, projMatrix_, col);
     M = Matrix4::Translation(Vector3(0,-0.2,0)) * Matrix4::Scale(Vector3(40, 1, 50));
     quickShapes_.DrawSquare(modelMatrix_ * M, viewMatrix_, projMatrix_, Color(1,1,1), fieldTex_);

     // Draw the car
     Color carcol(0.8, 0.2, 0.2);
     Matrix4 Mcar =
         Matrix4::Translation(car_.position() - Point3(0,0,0)) *
         Matrix4::RotationY(car_.angle()) *
         Matrix4::Scale(car_.size()) *
         Matrix4::Scale(Vector3(0.5,0.5,0.5));
     quickShapes_.DrawCube(modelMatrix_ * Mcar, viewMatrix_, projMatrix_, carcol);


     // Draw the ball
     Color ballcol(1,1,1);
     Matrix4 Mball =
         Matrix4::Translation(ball_.position() - Point3(0,0,0)) *
         Matrix4::Scale(Vector3(ball_.radius(), ball_.radius(), ball_.radius()));
     quickShapes_.DrawSphere(modelMatrix_ * Mball, viewMatrix_, projMatrix_, ballcol);


     // Draw the ball's shadow -- this is a bit of a hack, scaling Y by zero
     // flattens the sphere into a pancake, which we then draw just a bit
     // above the ground plane.
     Color shadowcol(0.2,0.4,0.15);
     Matrix4 Mshadow =
         Matrix4::Translation(Vector3(ball_.position()[0], -0.1, ball_.position()[2])) *
         Matrix4::Scale(Vector3(ball_.radius(), 0, ball_.radius())) *
         Matrix4::RotationX(90);
     quickShapes_.DrawSphere(modelMatrix_ * Mshadow, viewMatrix_, projMatrix_, shadowcol);


     // You should add drawing the goals and the boundary of the playing area
     // using quickShapes_.DrawLines()


     //draw the boundary
     std::vector<Point3> loop1;
     loop1.push_back(Point3( -40.0, 35.0, -50.0));
     loop1.push_back(Point3(-40.0, 0.0, -50.0));
     loop1.push_back(Point3(-40.0, 0.0,  50.0));
     loop1.push_back(Point3( -40.0, 35.0,  50.0));
     quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, Color(1,1,1), loop1, QuickShapes::LinesType::LINE_LOOP, 0.1);

     Matrix4 bound1 = Matrix4::Translation(Vector3(80,0, 0));
     quickShapes_.DrawLines(modelMatrix_*bound1, viewMatrix_, projMatrix_, Color(1,1,1), loop1, QuickShapes::LinesType::LINE_LOOP, 0.1);

     std::vector<Point3> loop2;
     loop2.push_back(Point3( -40.0, 35.0, -50.0));
     loop2.push_back(Point3(40.0, 35.0, -50.0));
     loop2.push_back(Point3(40.0, 0.0,-50.0));
     loop2.push_back(Point3( -40.0, 0.0, -50.0));
     quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, Color(1,1,1), loop2, QuickShapes::LinesType::LINE_LOOP, 0.1);

     Matrix4 bound2 = Matrix4::Translation(Vector3(0,0,100));
     quickShapes_.DrawLines(modelMatrix_*bound2, viewMatrix_, projMatrix_, Color(1,1,1), loop2, QuickShapes::LinesType::LINE_LOOP, 0.1);

     //Draw the goals
     Matrix4 transGoal = Matrix4::Translation(Vector3(0,0,-100));
     for (int i = -10; i < 10; i ++){
        for (int j =0; j < 10; j++){
            std::vector<Point3> goal;
            goal.push_back(Point3(i, j, 50) );
            goal.push_back(Point3(i, j + 1, 50) );
            goal.push_back(Point3(i + 1, j + 1, 50) );
            goal.push_back(Point3(i + 1, j, 50) );
            quickShapes_.DrawLines(modelMatrix_, viewMatrix_, projMatrix_, Color(1,1,0), goal, QuickShapes::LinesType::LINE_LOOP, 0.1);
            quickShapes_.DrawLines(modelMatrix_*transGoal, viewMatrix_, projMatrix_, Color(0,0,1), goal, QuickShapes::LinesType::LINE_LOOP, 0.1);
        }
    }

 }
