#define GLM_FORCE_RADIANS
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "model.h"

class BL2GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core 
{
  Q_OBJECT

  public:
    BL2GLWidget (QWidget *parent=0);
    ~BL2GLWidget ();

  protected:
    int printOglError(const char file[], int line, const char func[]);
  
    // initializeGL - Aqui incluim les inicialitzacions del contexte grafic.
    virtual void initializeGL ( );
    // paintGL - Mètode cridat cada cop que cal refrescar la finestra.
    // Tot el que es dibuixa es dibuixa aqui.
    virtual void paintGL ( );
    // resizeGL - És cridat quan canvia la mida del widget
    virtual void resizeGL (int width, int height);
    // keyPressEvent - Es cridat quan es prem una tecla
    virtual void keyPressEvent (QKeyEvent *event);
    // mouse{Press/Release/Move}Event - Són cridades quan es realitza l'event
    // corresponent de ratolí
    virtual void mousePressEvent (QMouseEvent *event);
    virtual void mouseReleaseEvent (QMouseEvent *event);
    virtual void mouseMoveEvent (QMouseEvent *event);

    // iniEscena - Aqui incluim les inicialitzacions de l'escena
    virtual void iniEscena ();
    // iniCamera - Aqui incluim les inicialitzacions de la camera
    virtual void iniCamera ();    
    // creaBuffersModels - Aquí carreguem els fitxers obj i fem la inicialització dels diferents VAOS i VBOs
    virtual void creaBuffersModels ();
	// creaBuffersTerra - Aqui inicialitzem el VAO i els VBO d'un model fet a mà que representa un cub
    virtual void creaBuffersCub ();
    // creaBuffersTerra - Aqui inicialitzem el VAO i els VBO d'un model fet a mà que representa un terra
    virtual void creaBuffersTerra ();
    // carregaShaders - Aquí carreguem els shaders, els compilem i els linkem. També busquem els uniform locations que fem servir.
    virtual void carregaShaders ();
    
    // viewTransform i projecTransform - Es fan servir per a construir i enviar als shader les matrius de càmera (View i Projection respectivament).
    virtual void viewTransform ();
    virtual void projectTransform ();
    
    // {Rick/VideoCamera/Cub/ident}Transform - Funcions per construir i enviar al shader diferents matrius de transformació geomètrica TG
    virtual void RickTransform();
	virtual void VideoCameraTransform();
	virtual void CubTransform();
    virtual void identTransform();
    
    // calculaCapsaModel - Calcula la capsa contenidora d'un Model p retornant el centre de la seva base a CentreBase, i el factor d'escala necessari per a que la seva alçada sigui alcadaDesitjada.
    virtual void calculaCapsaModel (Model &p, float &escala, float alcadaDesitjada, glm::vec3 &CentreBase);

    // attribute locations
    GLuint vertexLoc, colorLoc;

    // uniform locations
    GLuint transLoc, viewLoc, projLoc;

    // VAO names
    GLuint VAO_Rick;
	GLuint VAO_VideoCamera;
    GLuint VAO_Terra;
	GLuint VAO_Cub;

    // Program
    QOpenGLShaderProgram *program;

    // Viewport
    GLint ample, alt;

    // Mouse interaction
    typedef  enum {NONE, ROTATE} InteractiveAction;
    InteractiveAction DoingInteractive;
    int xClick, yClick;
    float factorAngleX, factorAngleY;

    // Internal vars
    float radiEscena;
    float fov, ra, znear, zfar;
    glm::vec3 centreEscena, obs, vrp, up;

    // Model, posició, escala i orientació del Rick i de la videocamera
    Model rick,videoCamera;
    glm::vec3 posRick=glm::vec3(0,0,-3);
    float alcadaRick=2, alcadaVideoCamera=1;
    float angleRick = M_PI/2, angleVideoCamera=M_PI/2;
	float escalaRick, escalaVideoCamera;
	glm::vec3 centreCaixaRick, centreCaixaVideoCamera;
	
	QTimer *timer;
	
	public slots:
	
	virtual void updatePosition();

};

