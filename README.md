# CS-330: Computer Graphics and Visualization  
OpenGL Credenza Scene With Capstone Enhancements

<div style="display: flex; flex-wrap: wrap; gap: 10px; justify-content: center;">

  <img src="https://github.com/Andereth000/CS-330-Computer-Graphics-And-Visualization/blob/main/Images/Final_Project_1.png?raw=true" alt="Project Preview 1" style="width: 32%; max-width: 400px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.2);">

  <img src="https://github.com/Andereth000/CS-330-Computer-Graphics-And-Visualization/blob/main/Images/Final_Project_2.png?raw=true" alt="Project Preview 2" style="width: 32%; max-width: 400px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.2);">

  <img src="https://github.com/Andereth000/CS-330-Computer-Graphics-And-Visualization/blob/main/Images/enhancement_1.png?raw=true" alt="Project Preview 3" style="width: 32%; max-width: 400px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.2);">

  <img src="https://github.com/Andereth000/CS-330-Computer-Graphics-And-Visualization/blob/main/Images/enhancement_2.png?raw=true" alt="Project Preview 4" style="width: 48%; max-width: 500px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.2);">

  <img src="https://github.com/Andereth000/CS-330-Computer-Graphics-And-Visualization/blob/main/Images/enhancement_3.png?raw=true" alt="Project Preview 5" style="width: 48%; max-width: 500px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.2);">

</div>

### 🔗 Required Files

The following files are required to build and run the project.  
They are not included in the GitHub repository due to size limitations.

**Download them from this Google Drive folder:**

➡️ [Download Libraries and DLLs](https://drive.google.com/drive/folders/1e7LxHdDf2Xe31Ot5mpjnGtcvM-U3H7L7?usp=sharing)

**Includes:**
- assimp.zip
- GLEW.zip
- ImGui.zip
- Models.zip

## 🛠️ Build Instructions – Credenza OpenGL Enhanced

Follow the steps below to build and run the enhanced OpenGL rendering engine with mesh editing, model loading, and scene saving and loading functionality.

1. **Clone the Repository**  
   Clone the repository from the `main` branch:

   ```bash
   git clone https://github.com/Andereth000/CS-330-Computer-Graphics-And-Visualization.git
   ```

2. **Add Required Library Dependencies**  
   **Files Required:** Provided in Google Drive folder

   Navigate to:

   ```
   \CS-330-Computer-Graphics-And-Visualization\Credenza_OpenGL\Libraries
   ```

   Replace the `GLEW` and `imgui` folders with the contents of the extracted `GLEW.zip` and `imgui.zip`.

   > (Note: Go up one directory from the extracted folder before copying.)

3. **Add the Models Folder**  
   **Files Required:** Included in Brightspace or external download

   Navigate to:

   ```
   \CS-330-Computer-Graphics-And-Visualization\Credenza_OpenGL
   ```

   Paste the `Models` folder into this directory.

4. **Copy Required DLL Files**  
   **Files Required:** Included in Brightspace or download link

   From:

   ```
   \GLEW\bin\Release\Win32
   ```

   Copy `glew32.dll` and paste it into:

   ```
   \Credenza_OpenGL\Projects\7-1 Final Project
   ```

   From:

   ```
   \assimp\bin
   ```

   Copy `assimp-vc143-mtd.dll` and paste it into the same directory.

5. **Open and Run in Visual Studio**  
   Navigate to:

   ```
   \Credenza_OpenGL\Projects\7-1 Final Project
   ```

   Open the solution:

   ```
   7-1_FinalProjectMilestone.sln
   ```

   Build and run the project using **Debug x86** or **Release x86** configuration.

