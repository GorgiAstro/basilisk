/*
 ISC License

 Copyright (c) 2022, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */

#include <cstring>
#include <iostream>
#include <cmath>

#include "constraintDynamicEffector.h"
#include "architecture/utilities/linearAlgebra.h"
#include "architecture/utilities/astroConstants.h"
#include "architecture/utilities/macroDefinitions.h"
#include "architecture/utilities/avsEigenSupport.h"
#include "architecture/utilities/rigidBodyKinematics.h"
#include <cstring>
#include <iostream>
#include <cmath>

/*! The Constructor.*/
ConstraintDynamicEffector::ConstraintDynamicEffector()
{
    // type of constraint to be implemented
    this->constraint_type = '';
    
    // counters and flags
    this->scInitCounter = 0.0;
    this->scCounterFlag = 1;

    // - Initialize constraint dimensions
    this->r_P1B1_B1.setZero();
    this->r_P2B2_B2.setZero();
    this->l = 0.0;
    this->rInit_P2P1_B1.setZero();

    // - Initialize gains
    this->alpha = 0.0;
    this->beta = 0.0;
    this->k = 0.0;
    this->c = 0.0;
    this->kI = -1;
    this->kI_att = -1;
    this->K = -1;
    this->P = -1;

    this->Fc_N.setZero();
    this->L_B2.setZero();

    return;
}

/*! The destructor. */
ConstraintDynamicEffector::~ConstraintDynamicEffector()
{
    return;
}


/*! This method is used to reset the module.
 @return void
 */
void ConstraintDynamicEffector::Reset(uint64_t CurrentSimNanos)
{
    this->kI = 0.0;
    this->kI_att = 0.0;
    this->k = pow(this->alpha,2);
    this->c = 2*this->beta;

    return;
}

/*! This method is used to link the states to the thrusters
 @return void
 @param states The states to link
 */
void ConstraintDynamicEffector::linkInStates(DynParamManager& states)
{
    if (this->scInitCounter > 1) {
        bskLogger.bskLog(BSK_ERROR, "constraintDynamicEffector: tried to attach more than 2 spacecraft");
    }

    this->hubSigma[this->scInitCounter] = states.getStateObject("hubSigma");
	this->hubOmega[this->scInitCounter] = states.getStateObject("hubOmega");
    this->hubPosition[this->scInitCounter] = states.getStateObject("hubPosition");
    this->hubVelocity[this->scInitCounter] = states.getStateObject("hubVelocity");

    scInitCounter++;
    return;
}

/*! This method computes the Forces on Torque on the Spacecraft Body.
 @return void
 @param integTime Integration time
 @param timeStep Current integration time step used
 */
void ConstraintDynamicEffector::computeForceTorque(double integTime, double timeStep)
{
    if (this->scCounterFlag == 1) {
        // flag = 1 signifies being called by spacecraft 1

        // - Collect states from both spacecraft
        Eigen::Vector3d r_B1N_N = this->hubPosition[0]->getState();
        Eigen::Vector3d rDot_B1N_N = this->hubVelocity[0]->getState();
        Eigen::Vector3d omega_B1N_B1 = this->hubOmega[0]->getState();
        Eigen::MRPd sigma_B1N = (Eigen::Vector3d) this->hubSigma[0]->getState();
        Eigen::Vector3d r_B2N_N = this->hubPosition[1]->getState();
        Eigen::Vector3d rDot_B2N_N = this->hubVelocity[1]->getState();
        Eigen::Vector3d omega_B2N_B2 = this->hubOmega[1]->getState();
        Eigen::MRPd sigma_B2N = (Eigen::Vector3d) this->hubOmega[1]->getState();

        // computing direction constraint psi in the N frame
        Eigen::Matrix3d dcm_B1N = (sigma_B1N.toRotationMatrix()).transpose();
        Eigen::Matrix3d dcm_B2N = (sigma_B2N.toRotationMatrix()).transpose();
        Eigen::Matrix3d dcm_NB1 = dcm_B1N.transpose();
        Eigen::Matrix3d dcm_NB2 = dcm_B2N.transpose();
        Eigen::Vector3d r_P1B1_N = dcm_NB1*this->r_P1B1_B1;
        Eigen::Vector3d r_P2B2_N = dcm_NB2*this->r_P2B2_B2;
        Eigen::Vector3d r_P2P1_N = r_P2B2_N + r_B2N_N - r_P1B1_N - r_B1N_N;
        Eigen::Vector3d psi_N = r_P2P1_N - dcm_NB1*this->rInit_P2P1_B1;
        
        // computing length constraint rate of change psiDot in the N frame
        Eigen::Vector3d rDot_P1B1_B1 = omega_B1N_B1.cross(r_P1B1_B1);
        Eigen::Vector3d rDot_P2B2_B2 = omega_B2N_B2.cross(r_P2B2_B2);
        Eigen::Vector3d rDot_P1N_N = rDot_B1N_N + dcm_NB1*rDot_P1B1_B1;
        Eigen::Vector3d rDot_P2N_N = rDot_B2N_N + dcm_NB2*rDot_P2B2_B2;
        Eigen::Vector3d rDot_P2P1_N = rDot_P2N_N - rDot_P1N_N;
        Eigen::Vector3d omega_B1N_N = dcm_NB1*omega_B1N_B1;
        Eigen::Vector3d psiPrime_N = rDot_P2P1_N - omega_B1N_N.cross(r_P2P1_N);

        // calculative the difference in angular rate
        Eigen::Vector3d omega_B1N_B2 = dcm_B2N*dcm_NB1*omega_B1N_B1;
        Eigen::Vector3d omega_B2B1_B2 = omega_B2N_B2 - omega_B1N_B2;

        // calculate the difference in attitude
        Eigen::MRPd sigma_B2B1 = eigenC2MRP(dcm_B2N*dcm_NB1);

        // computing the constraint force
        this->Fc_N = this->k*psi_N + this->c*psiPrime_N;
        this->forceExternal_N = this->Fc_N;

        // computing constraint torque from direction constraint
        Eigen::Vector3d Fc_B1 = dcm_B1N*this->Fc_N;
        Eigen::Vector3d Fc_B2 = dcm_B2N*this->Fc_N;
        this->torqueExternalPntB_B = this->r_P1B1_B1.cross(Fc_B1);
        Eigen::Vector3d L_B2_len = this->r_P2B2_B2.cross(Fc_B2);

        // computing constraint torque from attitude constraint
        Eigen::Vector3d L_B2_att = -this->K*sigma_B2B1-this->P*omega_B2B1_B2;

        // total torque imparted on spacecraft 2
        this->L_B2 = L_B2_len + L_B2_att;

    } else if (this->scCounterFlag == -1) {
        // flag = -1 signifies being called by spacecraft 2

        // computing the constraint force from stored magnitude
        this->forceExternal_N = -this->Fc_N;

        // computing constraint torque
        this->torqueExternalPntB_B = L_B2;

    }
    this->scCounterFlag *= -1; // switching between spacecraft calls
    
    return;
}

/*! This method is the main cyclical call for the scheduled part of the thruster
 dynamics model.  It reads the current commands array and sets the thruster
 configuration data based on that incoming command set.  Note that the main
 dynamical method (ComputeDynamics()) is not called here and is intended to be
 called from the dynamics plant in the system
 @return void
 @param CurrentSimNanos The current simulation time in nanoseconds
 */
void ConstraintDynamicEffector::UpdateState(uint64_t CurrentSimNanos)
{
    // TODO: Compute integral feedback term

}