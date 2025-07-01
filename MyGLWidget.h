#include "BL2GLWidget.h"

#include "model.h"

class MyGLWidget : public BL2GLWidget {
  Q_OBJECT

    public:
        MyGLWidget(QWidget *parent=0) : BL2GLWidget(parent) {}
        ~MyGLWidget();

    public slots:
      void toggleCamera();  //Slot per camButton
      void reset(); //Slot per resetButton
      void mouRickAuto();   //Slot pel timer de moviment automàtic
      void toggleAuto();    //Slot per autoButton
      void mouPorta();  //Slot pel timer d'animació de la porta

    protected:
        //Funcions de setup
        virtual void initializeGL() override;
        virtual void iniEscena() override;
        virtual void iniCamera() override;

        //Funció de pintat i redimensionat
        virtual void paintGL() override;
        virtual void resizeGL(int weight, int height) override;

        //Funcions de transformació
        virtual void RickTransform() override;
        virtual void VideoCameraTransform() override;
        virtual void CubTransform() override;

        //Funcions de transformació de càmera
        virtual void viewTransform() override;
        virtual void projectTransform() override;

        //Funcions d'input
        virtual void mousePressEvent (QMouseEvent *event) override;
        virtual void mouseReleaseEvent (QMouseEvent *event) override;
        virtual void mouseMoveEvent(QMouseEvent *e) override;
        virtual void keyPressEvent(QKeyEvent *event) override;

        //Variables d'escala i posició dels objectes
        glm::vec3 posParet1, posParet2, posPortaTancada, posPortaOberta, posPortaActual, posCamera;
        glm::vec3 escalaParet, escalaPorta;

        //Variables de càmera
        float psi, theta, d, fovIni, rav, fixCamFov;

        //Variable per diferenciar en pintaCub()
        int paretAct;

        //Variables per togglejar modes
        bool fixCam, autoMove;

        //Timer per animar la porta
        QTimer *portaTimer;

    private:
        //Attribute location
        GLint portaLoc;
};
