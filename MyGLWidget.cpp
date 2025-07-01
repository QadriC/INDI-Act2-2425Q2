#include "MyGLWidget.h"
#include <iostream>
#include <stdio.h>



MyGLWidget::~MyGLWidget()
{

}

void MyGLWidget::initializeGL() {
    portaTimer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(mouRickAuto()));
    connect(portaTimer, SIGNAL(timeout()), this, SLOT(mouPorta()));
    fixCam = false, autoMove = false;
    portaTimer->start(50); //output cada 200ms
    posParet1 = glm::vec3(0, 0, 2.5);
    posParet2 = glm::vec3(0, 0, -2.5);
    posPortaTancada = glm::vec3(0, 0, 0);
    posPortaOberta = glm::vec3(0, 0, 2);
    posPortaActual = posPortaTancada;
    posCamera = glm::vec3(0, 2.25, -1);
    escalaParet = glm::vec3(0.5f, 2.0f, 3.0f);
    escalaPorta = glm::vec3(0.25f, 2.0f, 2.0f);

    posRick = glm::vec3(-5, 0, 0);
    angleRick = float(M_PI)/2.0f;
    alcadaRick = 1.5f;

    alcadaVideoCamera = 0.5f;
    angleVideoCamera = 0.0f;

    BL2GLWidget::initializeGL();    //iniEscena(), iniCamera()
    portaLoc = glGetUniformLocation(program->programId(), "porta");
    setMouseTracking(true);
    fovIni = 2*asin(radiEscena / d);
    fixCamFov = float(M_PI) / 2.0f;
}

void MyGLWidget::iniEscena() {
    float alturaMax = 2.75f;
    centreEscena = glm::vec3(0.0f, alturaMax / 2.0f, 0.0f);
    glm::vec3 puntMin(-6.0f, 0.0f, -4.0f);
    glm::vec3 puntMax(6.0f, alturaMax, 4.0f);
    radiEscena = glm::distance(puntMin, puntMax) / 2.0f;
}

void MyGLWidget::iniCamera() {
    d = radiEscena * 2.0f;
    psi = theta = float(M_PI)/4.0f;
    znear = d - radiEscena;
    zfar  = d + radiEscena;
    projectTransform();
    viewTransform();
}

void MyGLWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
            xClick = event->x();
            yClick = event->y();
            DoingInteractive = ROTATE;
    }
}

void MyGLWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
            DoingInteractive = NONE;
    }
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *e) {
    if(!fixCam && DoingInteractive == ROTATE) {
        int newx = e->x();
        int newy = e->y();
        //std::cout << "x: " << newx << " y: " << newy << std::endl;

        psi -= (newx - xClick) * M_PI / 180.0f; //Es resta perquè a viewTransform() utilitzem -psi.
        theta += (newy - yClick) * M_PI / 180.0f;

        xClick = newx;
        yClick = newy;
        makeCurrent();  //Context actual perquè la càmara s'actualitzi correctament
        viewTransform();
        doneCurrent();
        update();
    }
}

void MyGLWidget::keyPressEvent(QKeyEvent *e) {
    bool moved = false;
    switch (e->key()) {
        case Qt::Key_Up:
            if((posRick.x + 1.0f <= 6.0f && !autoMove)){
                if((posRick.x > -2.0 && posRick.x < 2.0 && posPortaActual != posPortaOberta)) {
                    //No travessar la porta.
                } else {
                    posRick.x += 1.0f;
                    angleRick = float(M_PI) / 2.0f;
                    moved = true;
                }
            }
            break;
        case Qt::Key_Down:
            if((posRick.x - 1.0f >= -6.0f && !autoMove) || (posRick.x > -3.0 && posRick.x < 3.0 && posPortaActual == posPortaOberta)){
                if((posRick.x > -2.0 && posRick.x < 2.0 && posPortaActual != posPortaOberta)) {
                    //No travessar la porta.
                } else {
                    posRick.x -= 1.0f;
                    angleRick = -float(M_PI) / 2.0f;
                    moved = true;
                }
            }
            break;
        case Qt::Key_C:
            toggleCamera();
            break;
        case Qt::Key_R:
            reset();
            break;
        case Qt::Key_A:
            autoMove = !autoMove;
            if(autoMove)
                timer->start(200);
            else
                timer->stop();
            break;
    }

    if(moved) {
        makeCurrent();  //Context actual perquè la posició i angle s'actualitzin correctament
        viewTransform();
        projectTransform();
        doneCurrent();
        update();
    }
}

void MyGLWidget::projectTransform() {
    float fov;
    if(!fixCam) {
        fov = fovIni;
        znear = d - radiEscena;
        zfar = d+ radiEscena;
        if (rav < 1.0f)
            fov = 2.0f * atan(tan(fovIni / 2.0f) / rav);
    } else {
        fov = fixCamFov;
        znear = 0.1f;
        zfar = glm::distance(centreEscena, glm::vec3(6., 0., 4.));
        if (rav < 1.0f)
            fov = 2.0f * atan(tan(fixCamFov / 2.0f) / rav);
    }
    glm::mat4 Proj = glm::perspective(fov, rav, znear, zfar);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform (){
    glm::mat4 VM(1.0f);
    if(fixCam) {
        VM = glm::lookAt(posCamera, posRick, glm::vec3(0,1,0));
    } else {
        VM = glm::translate(VM, glm::vec3(0.,0.,-d));
        VM = glm::rotate(VM, theta, glm::vec3(1.,0.,0.));
        VM = glm::rotate(VM, -psi, glm::vec3(0.,1.,0.));
        VM= glm::translate(VM, -centreEscena);
    }
    glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &VM[0][0]);
}

void MyGLWidget::toggleCamera() {
    fixCam = !fixCam;
    makeCurrent();
    projectTransform();
    viewTransform();
    doneCurrent();
    update();
}

void MyGLWidget::reset() {
    posPortaActual = posPortaTancada;
    fixCam = false, autoMove = false;
    timer->stop();
    posRick = glm::vec3(-5, 0, 0);
    angleRick = float(M_PI)/2.0f;
    psi = theta = float(M_PI)/4.0f;
    makeCurrent();
    iniEscena();
    iniCamera();
    doneCurrent();
    update();
}
void MyGLWidget::toggleAuto() {
    autoMove = !autoMove;
    if(autoMove)
        timer->start(200);
    else
        timer->stop();
}

void MyGLWidget::mouRickAuto() {
    if(angleRick == float(M_PI) / 2.0f) {
        if(posRick.x + 1.0 <= 6.0f) {
            posRick.x += 1.0;
        } else {
            angleRick = -float(M_PI) / 2.0f;
        }
    } else {
        if(posRick.x - 1.0 >= -6.0f) {
            posRick.x -= 1.0;
        } else {
            angleRick = float(M_PI) / 2.0f;
        }
    }
    makeCurrent();
    viewTransform();
    projectTransform();
    doneCurrent();
    update();
}

void MyGLWidget::mouPorta() {
    if(posRick.x > -3.0 && posRick.x < 3.0) {
        //Obrir porta
        if(posPortaActual.z < posPortaOberta.z) {
            posPortaActual.z += 0.25f;
        }
    } else if(posPortaActual.z > posPortaTancada.z) {
        //Tancar porta
        posPortaActual.z -= 0.25f;
    }
    makeCurrent();
    projectTransform();
    viewTransform();
    doneCurrent();
    update();
}

void MyGLWidget::RickTransform() {
  glm::mat4 TG(1.0f);

  TG = glm::translate(TG, posRick);
  TG = glm::rotate(TG, angleRick, glm::vec3(0,1,0));
  TG = glm::scale(TG, glm::vec3(escalaRick, escalaRick, escalaRick));
  TG = glm::translate(TG, -centreCaixaRick);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::VideoCameraTransform() {
  glm::mat4 TG(1.0f);

  TG = glm::translate(TG, posCamera);
  TG = glm::rotate(TG, angleVideoCamera, glm::vec3(0, 1, 0));
  TG = glm::scale(TG, glm::vec3(escalaVideoCamera, escalaVideoCamera, escalaVideoCamera));
  TG = glm::translate(TG, -centreCaixaVideoCamera);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::CubTransform() {
  glm::mat4 TG(1.0f);

  if(paretAct == 1) {
      TG = glm::translate(TG, posParet1);
      TG = glm::scale(TG, escalaParet);
      glUniform1i(portaLoc, 0);
  } else if(paretAct == 2) {
      TG = glm::translate(TG, posParet2);
      TG = glm::scale(TG, escalaParet);
      glUniform1i(portaLoc, 0);
  } else if(paretAct == 3) {
      TG = glm::translate(TG, posPortaActual);
      TG = glm::scale(TG, escalaPorta);
      glUniform1i(portaLoc, 1);
  }

  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::resizeGL(int w, int h) {
    ample = w;
    alt = h;
    rav = float(ample) / float(alt);

    projectTransform();
    viewTransform();
    //glViewport(0, 0, ample, alt);
}

void MyGLWidget::paintGL() {
    // descomentar per canviar paràmetres
    glViewport (0, 0, ample, alt);

    // Esborrem el frame-buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Rick
    glBindVertexArray (VAO_Rick);
    RickTransform();
    glUniform1i(portaLoc, 0);
    glDrawArrays(GL_TRIANGLES, 0, rick.faces().size()*3);

    // VideoCamera
    if(!fixCam) {
        glBindVertexArray (VAO_VideoCamera);
        VideoCameraTransform();
        glUniform1i(portaLoc, 0);
        glDrawArrays(GL_TRIANGLES, 0, videoCamera.faces().size()*3);
    }

    // Paret 1
    glBindVertexArray (VAO_Cub);
    paretAct = 1;
    CubTransform();
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Paret 2
    glBindVertexArray (VAO_Cub);
    paretAct = 2;
    CubTransform();
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Porta
    glBindVertexArray (VAO_Cub);
    paretAct = 3;
    CubTransform();
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Terra
    glBindVertexArray (VAO_Terra);
    identTransform();
    glUniform1i(portaLoc, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray (0);
}
