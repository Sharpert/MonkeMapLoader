#include "Behaviours/Player.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils-fields.hpp"

DEFINE_CLASS(MapLoader::Player);

extern Logger& getLogger();

namespace MapLoader
{
    void Player::Start()
    {
        instance = this;
    }

    void Player::TeleportPlayer(Il2CppObject* destination)
    {
        if (instance->isTeleporting)
            return;
        getLogger().info("Teleporting Player!");

        Il2CppObject* destGO = *il2cpp_utils::RunMethod(destination, "get_gameObject");
        Il2CppString* goName = *il2cpp_utils::RunMethod<Il2CppString*>(destGO, "get_name");
        std::string name = to_utf8(csstrtostr(goName));

        getLogger().info("Teleporting to Object %s", name.c_str());

        instance->isTeleporting = true;
        instance->destination = destination;
        static std::vector<Il2CppClass*> rigidBodyKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Rigidbody")};
        Il2CppObject* go = *il2cpp_utils::RunMethod(instance, "get_gameObject");
        Il2CppObject* playerRigidBody = *il2cpp_utils::RunGenericMethod(go, "GetComponent", rigidBodyKlass);

        if (playerRigidBody)
        { 
            static std::vector<Il2CppClass*> playerKlass = {il2cpp_utils::GetClassFromName("GorillaLocomotion", "Player")};
            Il2CppObject* player = *il2cpp_utils::RunGenericMethod(go, "GetComponent", playerKlass);
            static Vector3 zero = {0.0f, 0.0f, 0.0f};
            il2cpp_utils::RunMethod(playerRigidBody, "set_velocity", zero);
            il2cpp_utils::RunMethod(playerRigidBody, "set_isKinematic", true);

            Il2CppObject* playerTransform = *il2cpp_utils::RunMethod(player, "get_transform");
            Il2CppObject* bodyCollider = *il2cpp_utils::GetFieldValue(player, "bodyCollider");
            Il2CppObject* colliderTransform = *il2cpp_utils::RunMethod(bodyCollider, "get_transform");
            Vector3 pos = *il2cpp_utils::RunMethod<Vector3>(destination, "get_position");
            Vector3 playerPos = *il2cpp_utils::RunMethod<Vector3>(playerTransform, "get_position");
            Vector3 colliderPos = *il2cpp_utils::RunMethod<Vector3>(colliderTransform, "get_position");
            float x = pos.x - colliderPos.x + playerPos.x;
            float y = pos.y - colliderPos.y + playerPos.y;
            float z = pos.z - colliderPos.z + playerPos.z;
            Vector3 correctedPosition = {x, y, z};

            getLogger().info("dest: %.2f, %.2f, %.2f", correctedPosition.x, correctedPosition.y, correctedPosition.z);
            il2cpp_utils::RunMethod(playerTransform, "set_position", correctedPosition);
            
            Quaternion destOrig = *il2cpp_utils::RunMethod<Quaternion>(destination, "get_rotation");
            Vector3 destEuler = *il2cpp_utils::RunMethod<Vector3>(destOrig, "get_eulerAngles");

            Il2CppObject* headCollider = *il2cpp_utils::GetFieldValue(player, "headCollider");
            Il2CppObject* headTransform = *il2cpp_utils::RunMethod(headCollider, "get_transform");
            Quaternion headRot = *il2cpp_utils::RunMethod<Quaternion>(headTransform, "get_rotation");
            Vector3 headEuler = *il2cpp_utils::RunMethod<Vector3>(headRot, "get_eulerAngles");
            float turnAmt = destEuler.y - headEuler.y;
            il2cpp_utils::RunMethod(player, "Turn", turnAmt);

            //Quaternion newrot = *il2cpp_utils::RunMethod<Quaternion>("UnityEngine", "Quaternion", "Euler", euler.x, euler.y, euler.z);
            ///il2cpp_utils::RunMethod(playerTransform, "set_rotation", newrot);

            // set last position
            il2cpp_utils::SetFieldValue(player, "lastPosition", correctedPosition);

            // reset velocity history
            int velocityHistorySize = *il2cpp_utils::GetFieldValue<int>(player, "velocityHistorySize");
            Array<Vector3>* velocityHistory = reinterpret_cast<Array<Vector3>*>(il2cpp_functions::array_new(classof(Vector3), velocityHistorySize));
            il2cpp_utils::SetFieldValue(player, "velocityHistory", velocityHistory);

            // set last head pos
            Vector3 headPos = *il2cpp_utils::RunMethod<Vector3>(headTransform, "get_position");
            il2cpp_utils::SetFieldValue(player, "lastHeadPosition", headPos);

            // set last left hand pos
            Vector3 lastLeftHandPos = *il2cpp_utils::RunMethod<Vector3>(player, "CurrentLeftHandPosition");
            il2cpp_utils::SetFieldValue(player, "lastLeftHandPosition", lastLeftHandPos);

            // set last right hand pos
            Vector3 lastRightHandPos = *il2cpp_utils::RunMethod<Vector3>(player, "CurrentRightHandPosition");
            il2cpp_utils::SetFieldValue(player, "lastRightHandPosition", lastRightHandPos);

            // set velocities to zero
            il2cpp_utils::SetFieldValue(player, "currentVelocity", zero);
            il2cpp_utils::SetFieldValue(player, "denormalizedVelocityAverage", zero);

            // we done pog
            il2cpp_utils::RunMethod(playerRigidBody, "set_isKinematic", false);
        }
        instance->isTeleporting = false;
    }
}