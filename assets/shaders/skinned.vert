#version 330 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in ivec4 aBoneIds;
layout(location=4) in vec4 aBoneWeights;

const int MAX_BONES = 100;
uniform mat4 model, view, proj;
uniform mat4 jointMatrices[MAX_BONES];
uniform bool enableSkinning = true;

out VS_OUT{
    vec3 Pos;
    vec3 N;
    vec2 UV;
} vs;

void main() {
    // First use original vertex position to be safe
    vec4 position = vec4(aPos, 1.0);
    vec3 normal = aNormal;
    
    // Apply skinning if enabled
    if (enableSkinning) {
        // Initialize with zero
        position = vec4(0.0);
        normal = vec3(0.0);
        float totalWeight = 0.0;
        
        // Apply bone transformations
        for(int i = 0; i < 4; i++) {
            float weight = aBoneWeights[i];
            if(weight > 0.0) {
                totalWeight += weight;
                int boneId = aBoneIds[i];
                
                // Transform position by bone matrix
                position += weight * jointMatrices[boneId] * vec4(aPos, 1.0);
                
                // Transform normal by bone matrix (ignoring translation)
                mat3 boneMat3 = mat3(jointMatrices[boneId]);
                normal += weight * boneMat3 * aNormal;
            }
        }
        
        // Normalize the result if we got valid weights
        if(totalWeight > 0.0) {
            position /= totalWeight;
            normal = normalize(normal);
        } else {
            // Fallback to original position
            position = vec4(aPos, 1.0);
            normal = aNormal;
        }
    }
    
    // Apply model transformation
    vec4 worldPos = model * position;
    vs.Pos = worldPos.xyz;
    vs.N = mat3(transpose(inverse(model))) * normal;
    vs.UV = aUV;
    
    gl_Position = proj * view * worldPos;
}
