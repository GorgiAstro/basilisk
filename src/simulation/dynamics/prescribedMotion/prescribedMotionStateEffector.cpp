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

#include "prescribedMotionStateEffector.h"
#include "architecture/utilities/avsEigenSupport.h"
#include "architecture/utilities/rigidBodyKinematics.h"
#include "architecture/utilities/avsEigenSupport.h"
#include "architecture/utilities/macroDefinitions.h"
#include <iostream>
#include <string>

/*! This is the constructor, setting variables to default values */
PrescribedMotionStateEffector::PrescribedMotionStateEffector()
{
    // Zero the mass props and mass prop rates contributions
    this->effProps.mEff = 0.0;
    this->effProps.rEff_CB_B.fill(0.0);
    this->effProps.IEffPntB_B.fill(0.0);
    this->effProps.rEffPrime_CB_B.fill(0.0);
    this->effProps.IEffPrimePntB_B.fill(0.0);

    // Initialize variables to working values
    this->mass = 0.0;
    this->IHubBc_B.Identity();
    this->IPntFc_F.Identity();
    this->r_FM_MInit = {1.0, 0.0, 0.0};
    this->rPrime_FM_MInit.setZero();
    this->rPrimePrime_FM_MInit.setZero();
    this->dcm_BF.Identity();
    this->dcm_BM.Identity();
    this->dcm_F0B.Identity();
    this->effectorID++; 
    return;
}

uint64_t PrescribedMotionStateEffector::effectorID = 1;

/*! This is the destructor, nothing to report here */
PrescribedMotionStateEffector::~PrescribedMotionStateEffector()
{
    this->effectorID = 1;    /* reset the panel ID*/

    return;
}

/*! This method is used to reset the module. */
void PrescribedMotionStateEffector::Reset(uint64_t CurrentClock)
{
    return;
}


/*! This method takes the computed theta states and outputs them to the messaging system. */
void PrescribedMotionStateEffector::writeOutputStateMessages(uint64_t CurrentClock)
{
    // Write out the state effector output messages
    if (this->prescribedMotionOutMsg.isLinked())
    {
        PrescribedMotionMsgPayload prescribedMotionBuffer;
        prescribedMotionBuffer = this->prescribedMotionOutMsg.zeroMsgPayload;
        eigenVector3d2CArray(this->r_FM_M, prescribedMotionBuffer.r_FM_M);
        eigenVector3d2CArray(this->rPrime_FM_M, prescribedMotionBuffer.rPrime_FM_M);
        eigenVector3d2CArray(this->rPrimePrime_FM_M, prescribedMotionBuffer.rPrimePrime_FM_M);
        eigenVector3d2CArray(this->omega_FB_F, prescribedMotionBuffer.omega_FB_F);
        eigenVector3d2CArray(this->omegaPrime_FB_F, prescribedMotionBuffer.omegaPrime_FB_F);
        eigenVector3d2CArray(this->sigma_FB, prescribedMotionBuffer.sigma_FB);
        this->prescribedMotionOutMsg.write(&prescribedMotionBuffer, this->moduleID, CurrentClock);
    }

    // Write out the prescribed effector body config log message
    if (this->prescribedMotionConfigLogOutMsg.isLinked())
    {
        SCStatesMsgPayload configLogMsg;
        configLogMsg = this->prescribedMotionConfigLogOutMsg.zeroMsgPayload;

        // Logging the F frame in the body frame B of that object
        eigenVector3d2CArray(this->r_FcN_N, configLogMsg.r_BN_N);
        eigenVector3d2CArray(this->v_FcN_N, configLogMsg.v_BN_N);
        eigenVector3d2CArray(this->sigma_FN, configLogMsg.sigma_BN);
        eigenVector3d2CArray(this->omega_FN_F, configLogMsg.omega_BN_B);
        this->prescribedMotionConfigLogOutMsg.write(&configLogMsg, this->moduleID, CurrentClock);
    }
    return;
}

/*! This method prepends the name of the spacecraft for multi-spacecraft simulations.*/
void PrescribedMotionStateEffector::prependSpacecraftNameToStates()
{
    return;
}

/*! This method allows the SB state effector to have access to the hub states and gravity*/
void PrescribedMotionStateEffector::linkInStates(DynParamManager& statesIn)
{
    // - Get access to the hub's sigma_BN, omegaBN_B and velocity needed for dynamic coupling and gravity
    std::string tmpMsgName;
    tmpMsgName = this->nameOfSpacecraftAttachedTo + "centerOfMassSC";
    this->c_B = statesIn.getPropertyReference(tmpMsgName);
    tmpMsgName = this->nameOfSpacecraftAttachedTo + "centerOfMassPrimeSC";
    this->cPrime_B = statesIn.getPropertyReference(tmpMsgName);

    this->hubSigma = statesIn.getStateObject(this->nameOfSpacecraftAttachedTo + "hubSigma");
    this->hubOmega = statesIn.getStateObject(this->nameOfSpacecraftAttachedTo + "hubOmega");
    this->hubPosition = statesIn.getStateObject(this->nameOfSpacecraftAttachedTo + "hubPosition");
    this->hubVelocity = statesIn.getStateObject(this->nameOfSpacecraftAttachedTo + "hubVelocity");

    return;
}

/*! This method allows the state effector to register its states with the dynamic parameter manager */
void PrescribedMotionStateEffector::registerStates(DynParamManager& states)
{
    return;
}

/*! This method allows the effector state effector to provide its contributions to the mass props and mass prop rates of the
 spacecraft */
void PrescribedMotionStateEffector::updateEffectorMassProps(double integTime)
{
    // Give the mass of the prescribed body to the effProps mass
    this->effProps.mEff = this->mass;

    // Call function to compute prescribed parameters and dcm_BF
    this->computePrescribedParameters(integTime);

    // Compute the effector's CoM with respect to point B
    this->r_FM_B = this->dcm_BM * this->r_FM_M;
    this->r_FB_B = this->r_FM_B + this->r_MB_B;
    this->r_FcF_B = this->dcm_BF * this->r_FcF_F;
    this->r_FcB_B = this->r_FcF_B + this->r_FB_B;
    this->effProps.rEff_CB_B = this->r_FcB_B;

    // Find the inertia of the effector about point B
    this->rTilde_FcB_B = eigenTilde(this->r_FcB_B);
    this->IPntFc_B = this->dcm_BF * this->IPntFc_F * this->dcm_BF.transpose();
    this->effProps.IEffPntB_B = this->IPntFc_B - this->mass * this->rTilde_FcB_B * this->rTilde_FcB_B;

    // Find rPrime_FcB_B
    this->omegaTilde_FB_B = eigenTilde(this->omega_FB_B);
    this->rPrime_FM_B = this->dcm_BM * this->rPrime_FM_M;
    this->rPrime_FcB_B = this->omegaTilde_FB_B * this->r_FcF_B + this->rPrime_FM_B;
    this->effProps.rEffPrime_CB_B = this->rPrime_FcB_B;

    // Find body time derivative of IPntFc_B
    Eigen::Matrix3d rPrimeTilde_FcB_B = eigenTilde(this->rPrime_FcB_B);
    this->effProps.IEffPrimePntB_B = this->omegaTilde_FB_B* this->IPntFc_B - this->IPntFc_B * this->omegaTilde_FB_B
        + this->mass * (rPrimeTilde_FcB_B * this->rTilde_FcB_B.transpose() + this->rTilde_FcB_B * rPrimeTilde_FcB_B.transpose());

    return;
}

/*! This method allows the state effector to give its contributions to the matrices needed for the back-sub
 method */
void PrescribedMotionStateEffector::updateContributions(double integTime, BackSubMatrices & backSubContr, Eigen::Vector3d sigma_BN, Eigen::Vector3d omega_BN_B, Eigen::Vector3d g_N)
{
    // Find the DCM from N to B frames
    this->sigma_BN = sigma_BN;
    this->dcm_BN = (this->sigma_BN.toRotationMatrix()).transpose();

    // Define omega_BN_B
    this->omega_BN_B = omega_BN_B;
    this->omegaTilde_BN_B = eigenTilde(this->omega_BN_B);

    // Define omegaPrimeTilde_FB_B
    Eigen::Matrix3d omegaPrimeTilde_FB_B = eigenTilde(this->omegaPrime_FB_B);

    // Define rPrimePrime_FcB_B
    this->rPrimePrime_FM_B = this->dcm_BM * this->rPrimePrime_FM_M;
    this->rPrimePrime_FcB_B = (omegaPrimeTilde_FB_B + this->omegaTilde_FB_B * this->omegaTilde_FB_B) * this->r_FcF_B + this->rPrimePrime_FM_B;

    // Translation contributions
    backSubContr.vecTrans = -this->mass * this->rPrimePrime_FcB_B;

    // Rotation contributions
    Eigen::Matrix3d IPrimePntFc_B = this->omegaTilde_FB_B * this->IPntFc_B - this->IPntFc_B * this->omegaTilde_FB_B;
    backSubContr.vecRot = - (this->mass * this->rTilde_FcB_B * this->rPrimePrime_FcB_B)  - (IPrimePntFc_B + this->omegaTilde_BN_B * this->IPntFc_B ) * this->omega_FB_B - this->IPntFc_B * this->omegaPrime_FB_B - this->mass * this->omegaTilde_BN_B * rTilde_FcB_B * this->rPrime_FcB_B;

    return;
}

/*! This method is empty because the equations of motion of the effector are prescribed */
void PrescribedMotionStateEffector::computeDerivatives(double integTime, Eigen::Vector3d rDDot_BN_N, Eigen::Vector3d omegaDot_BN_B, Eigen::Vector3d sigma_BN)
{
    return;
}

/*! This method is for calculating the contributions of the effector to the energy and momentum of the spacecraft */
void PrescribedMotionStateEffector::updateEnergyMomContributions(double integTime, Eigen::Vector3d & rotAngMomPntCContr_B, double & rotEnergyContr, Eigen::Vector3d omega_BN_B)
{
    // Update omega_BN_B and omega_FN_B
    this->omega_BN_B = omega_BN_B;
    this->omegaTilde_BN_B = eigenTilde(this->omega_BN_B);
    this->omega_FN_B = this->omega_FB_B + this->omega_BN_B;

    // Compute rDot_FcB_B
    this->rDot_FcB_B = this->rPrime_FcB_B + this->omegaTilde_BN_B * this->r_FcB_B;

    // Find rotational angular momentum contribution from hub
    rotAngMomPntCContr_B = this->IPntFc_B * this->omega_FN_B + this->mass * this->rTilde_FcB_B * this->rDot_FcB_B;

    // Find rotational energy contribution from the hub
    rotEnergyContr = 0.5 * this->omega_FN_B.dot(this->IPntFc_B * this->omega_FN_B) + 0.5 * this->mass * this->rDot_FcB_B.dot(this->rDot_FcB_B);

    return;
}

/*! This method computes the effector states relative to the inertial frame */
void PrescribedMotionStateEffector::computePrescribedMotionInertialStates()
{
    // inertial attitude
    Eigen::Matrix3d dcm_FN;
    dcm_FN = (this->dcm_BF).transpose() * this->dcm_BN;
    this->sigma_FN = eigenMRPd2Vector3d(eigenC2MRP(dcm_FN));

    // inertial position vector
    this->r_FcN_N = (Eigen::Vector3d)this->hubPosition->getState() + this->dcm_BN.transpose() * this->r_FcB_B;

    // inertial velocity vector
    this->v_FcN_N = (Eigen::Vector3d)this->hubVelocity->getState() + this->dcm_BN.transpose() * this->rDot_FcB_B;

    return;
}

/*! This method is used so that the simulation will ask the effector to update messages */ // called at dynamic freq
void PrescribedMotionStateEffector::UpdateState(uint64_t CurrentSimNanos)
{
    if (this->prescribedMotionInMsg.isLinked() && this->prescribedMotionInMsg.isWritten()) {
        PrescribedMotionMsgPayload incomingPrescribedStates;
        incomingPrescribedStates = this->prescribedMotionInMsg();
        this->r_FM_M = incomingPrescribedStates.r_FM_M;
        this->rPrime_FM_M = incomingPrescribedStates.rPrime_FM_M;
        this->rPrimePrime_FM_M = incomingPrescribedStates.rPrimePrime_FM_M;
        this->omega_FB_F = incomingPrescribedStates.omega_FB_F;
        this->omegaPrime_FB_F = incomingPrescribedStates.omegaPrime_FB_F;
        this->sigma_FB = incomingPrescribedStates.sigma_FB;


    /* Compute prescribed body inertial states */
    this->computePrescribedMotionInertialStates();

    this->writeOutputStateMessages(CurrentSimNanos);

    return;
}