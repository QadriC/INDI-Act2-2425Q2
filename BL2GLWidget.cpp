#include "BL2GLWidget.h"

#include <iostream>

#define printOpenGLError() printOglError(__FILE__, __LINE__)
#define CHECK() printOglError(__FILE__, __LINE__,__FUNCTION__)
#define DEBUG() std::cout << __FILE__ << " " << __LINE__ << " " << __FUNCTION__ << std::endl;

int BL2GLWidget::printOglError(const char file[], int line, const char func[]) 
{
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    const char * error = 0;
    switch (glErr)
    {
        case 0x0500:
            error = "GL_INVALID_ENUM";
            break;
        case 0x501:
            error = "GL_INVALID_VALUE";
            break;
        case 0x502: 
            error = "GL_INVALID_OPERATION";
            break;
        case 0x503:
            error = "GL_STACK_OVERFLOW";
            break;
        case 0x504:
            error = "GL_STACK_UNDERFLOW";
            break;
        case 0x505:
            error = "GL_OUT_OF_MEMORY";
            break;
        default:
            error = "unknown error!";
    }
    if (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s function: %s\n",
                             file, line, error, func);
        retCode = 1;
    }
    return retCode;
}

BL2GLWidget::BL2GLWidget (QWidget* parent) : QOpenGLWidget(parent), program(NULL)
{
  setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
  xClick = yClick = 0;
  DoingInteractive = NONE;
  timer=new QTimer(this);
  
  connect(timer, SIGNAL(timeout()), this, SLOT(updatePosition()));
}

BL2GLWidget::~BL2GLWidget ()
{
  if (program != NULL)
    delete program;
}

void BL2GLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();  
  glEnable(GL_DEPTH_TEST);
  
  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  creaBuffersModels();
  creaBuffersTerra();
  creaBuffersCub();
  iniEscena();
  iniCamera();
}

void BL2GLWidget::iniEscena()
{
  radiEscena = sqrt(80);
  centreEscena = glm::vec3(0,0,0);
}

void BL2GLWidget::iniCamera(){

  obs = glm::vec3(10, 5, 0);
  vrp = glm::vec3(0, 0, 0);
  up = glm::vec3(0, 1, 0);
  fov = float(M_PI)/4.0f;
  znear =  1;
  zfar  = 30;

  viewTransform();
}

void BL2GLWidget::updatePosition()
{
}

void BL2GLWidget::paintGL ()
{
  // descomentar per canviar paràmetres
  // glViewport (0, 0, ample, alt);

  // Esborrem el frame-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Rick
  glBindVertexArray (VAO_Rick);
  RickTransform();
  glDrawArrays(GL_TRIANGLES, 0, rick.faces().size()*3);
  
  // VideoCamera
  glBindVertexArray (VAO_VideoCamera);
  VideoCameraTransform();
  glDrawArrays(GL_TRIANGLES, 0, videoCamera.faces().size()*3);

  // Cub
  glBindVertexArray (VAO_Cub);
  CubTransform();
  glDrawArrays(GL_TRIANGLES, 0, 36);  
  
  // Terra
  glBindVertexArray (VAO_Terra);
  identTransform();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


  glBindVertexArray (0);
}

void BL2GLWidget::RickTransform ()
{
  glm::mat4 TG(1.0f);  
 
  TG = glm::translate(TG, posRick);
  TG=glm::rotate(TG, angleRick, glm::vec3(0,1,0));
  TG=glm::scale(TG, glm::vec3(escalaRick, escalaRick, escalaRick));
  TG=glm::translate(TG, -centreCaixaRick);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void BL2GLWidget::VideoCameraTransform ()
{
  glm::mat4 TG(1.0f);  
  TG=glm::rotate(TG, angleVideoCamera, glm::vec3(0,1,0));
  TG=glm::scale(TG, glm::vec3(escalaVideoCamera, escalaVideoCamera, escalaVideoCamera));
  TG=glm::translate(TG, -centreCaixaVideoCamera);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}


void BL2GLWidget::CubTransform ()
{
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, glm::vec3 (0, 0, 3));
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void BL2GLWidget::identTransform ()
{
  glm::mat4 TG(1.0f);
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void BL2GLWidget::projectTransform ()
{
  glm::mat4 Proj(1.0f);
  Proj = glm::perspective (fov, ra, znear, zfar);
  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void BL2GLWidget::viewTransform ()
{
  // Matriu de posició i orientació de l'observador
  glm::mat4 View(1.0f);
  View = glm::lookAt (obs, vrp, up);
  glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}


void BL2GLWidget::resizeGL (int w, int h) 
{
// Aquest codi és necessari únicament per a MACs amb pantalla retina.
#ifdef __APPLE__
  GLint vp[4];
  glGetIntegerv (GL_VIEWPORT, vp);
  ample = vp[2];
  alt = vp[3];
#else
  ample = w;
  alt = h;
#endif
  ra = float(ample)/float(alt);
  factorAngleY = M_PI / ample;
  factorAngleX = M_PI / alt;
  projectTransform();
}

void BL2GLWidget::keyPressEvent(QKeyEvent* event) 
{
  makeCurrent();
  update();
}

void BL2GLWidget::mousePressEvent (QMouseEvent *e)
{
  xClick = e->x();
  yClick = e->y();

  if (e->button() & Qt::LeftButton &&
      ! (e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
  {
    DoingInteractive = ROTATE;
  }
}

void BL2GLWidget::mouseReleaseEvent( QMouseEvent *)
{
  DoingInteractive = NONE;
}

void BL2GLWidget::mouseMoveEvent(QMouseEvent *e)
{
  makeCurrent();

  xClick = e->x();
  yClick = e->y();

  update ();
}

void BL2GLWidget::creaBuffersTerra ()
{
  // VBO amb la posició dels vèrtexs
  glm::vec3 posTerra[4] = {
        glm::vec3(-6.0, 0.0, -4.0),
        glm::vec3(-6.0, 0.0,  4.0),
        glm::vec3( 6.0, 0.0, -4.0),
        glm::vec3( 6.0, 0.0,  4.0)
  }; 

  glm::vec3 c(0.8, 0.7, 1.0);
  glm::vec3 colTerra[4] = { c, c, c, c };

  // VAO
  glGenVertexArrays(1, &VAO_Terra);
  glBindVertexArray(VAO_Terra);

  GLuint VBO_Terra[2];
  glGenBuffers(2, VBO_Terra);

  // geometria
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(posTerra), posTerra, GL_STATIC_DRAW);
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // color
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colTerra), colTerra, GL_STATIC_DRAW);
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);
  
  glBindVertexArray (0);
}

void BL2GLWidget::creaBuffersCub ()
{
  // Dades del cub
  // Vèrtexs del cub
  glm::vec3 vertexs[8] = {
       /* 0*/ glm::vec3( -0.5, 0.0, -0.5),  /* 1*/ glm::vec3( 0.5, 0.0, -0.5),
       /* 2*/ glm::vec3( -0.5, 1.0, -0.5),  /* 3*/ glm::vec3( 0.5, 1.0, -0.5),
       /* 4*/ glm::vec3( -0.5, 0.0, 0.5),  /* 5*/ glm::vec3( 0.5, 0.0, 0.5),
       /* 6*/ glm::vec3( -0.5, 1.0, 0.5),  /* 7*/ glm::vec3( 0.5, 1.0, 0.5)
  };

  // VBO amb la posició dels vèrtexs
  glm::vec3 poscub[36] = {  // 12 triangles
       vertexs[0], vertexs[2], vertexs[1],
       vertexs[1], vertexs[2], vertexs[3],
       vertexs[5], vertexs[1], vertexs[7],
       vertexs[1], vertexs[3], vertexs[7],
       vertexs[2], vertexs[6], vertexs[3],
       vertexs[3], vertexs[6], vertexs[7],
       vertexs[0], vertexs[4], vertexs[6],
       vertexs[0], vertexs[6], vertexs[2],
       vertexs[0], vertexs[1], vertexs[4],
       vertexs[1], vertexs[5], vertexs[4],
       vertexs[4], vertexs[5], vertexs[6],
       vertexs[5], vertexs[7], vertexs[6]
  };
  
  glm::vec3 color(0, 0, 1.0);

  glm::vec3 colorcub[36] = {
        color, color, color, color, color, color,
        color, color, color, color, color, color,
        color, color, color, color, color, color,
        color, color, color, color, color, color,
        color, color, color, color, color, color,
        color, color, color, color, color, color
  };
  
  // Creació del Vertex Array Object del cub
  glGenVertexArrays(1, &VAO_Cub);
  glBindVertexArray(VAO_Cub);

  GLuint VBO_Cub[2];
  glGenBuffers(2, VBO_Cub);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(poscub), poscub, GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_Cub[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colorcub), colorcub, GL_STATIC_DRAW);

  // Activem l'atribut normaBLoc
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);

  glBindVertexArray(0);
}

void BL2GLWidget::calculaCapsaModel (Model &p, float &escala, float alcadaDesitjada, glm::vec3 &centreBase)
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = p.vertices()[0];
  miny = maxy = p.vertices()[1];
  minz = maxz = p.vertices()[2];
  for (unsigned int i = 3; i < p.vertices().size(); i+=3)
  {
    if (p.vertices()[i+0] < minx)
      minx = p.vertices()[i+0];
    if (p.vertices()[i+0] > maxx)
      maxx = p.vertices()[i+0];
    if (p.vertices()[i+1] < miny)
      miny = p.vertices()[i+1];
    if (p.vertices()[i+1] > maxy)
      maxy = p.vertices()[i+1];
    if (p.vertices()[i+2] < minz)
      minz = p.vertices()[i+2];
    if (p.vertices()[i+2] > maxz)
      maxz = p.vertices()[i+2];
  }
  escala = alcadaDesitjada/(maxy-miny);
  centreBase[0] = (minx+maxx)/2.0; centreBase[1] = miny; centreBase[2] = (minz+maxz)/2.0;
}

void BL2GLWidget::creaBuffersModels ()
{
  // Càrrega dels models
  rick.load("./models/Rick.obj");
  videoCamera.load("./models/VideoCamera.obj");

  // Creació de VAOs i VBOs pel Ricky
  glGenVertexArrays(1, &VAO_Rick);
  	
  // Calculem la capsa contenidora del model
  calculaCapsaModel(rick, escalaRick, alcadaRick, centreCaixaRick);
  glBindVertexArray(VAO_Rick);

  GLuint VBO[2];
  glGenBuffers(2, VBO);

  // geometria
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*rick.faces().size()*3*3,
	   rick.VBO_vertices(), GL_STATIC_DRAW);
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // color
  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*rick.faces().size()*3*3,
	   rick.VBO_matdiff(), GL_STATIC_DRAW);
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);  
  

  glGenBuffers(2, VBO);
  
  // Creació de VAOs i VBOs per la càmera
  glGenVertexArrays(1, &VAO_VideoCamera);
  	
  // Calculem la capsa contenidora del model
  calculaCapsaModel(videoCamera, escalaVideoCamera, alcadaVideoCamera, centreCaixaVideoCamera);
  glBindVertexArray(VAO_VideoCamera);

  // geometria
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*videoCamera.faces().size()*3*3,
	   videoCamera.VBO_vertices(), GL_STATIC_DRAW);
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // color
  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*videoCamera.faces().size()*3*3,
	   videoCamera.VBO_matdiff(), GL_STATIC_DRAW);
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);  
  
  
  glBindVertexArray (0);
}

void BL2GLWidget::carregaShaders()
{
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile("shaders/basicShader.frag");
  vs.compileSourceFile("shaders/basicShader.vert");
  // Creem el program
  program = new QOpenGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);
  // Linkem el program
  program->link();
  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Identificador per als  atributs
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  colorLoc = glGetAttribLocation (program->programId(), "color");

  // Identificadors dels uniform locations
  transLoc = glGetUniformLocation(program->programId(), "TG");
  projLoc  = glGetUniformLocation (program->programId(), "Proj");
  viewLoc  = glGetUniformLocation (program->programId(), "View");
}


