---
hide:
  - toc
---
 <style>
  .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>

### Upload files
 
* To upload your patch on Github, first you need a Github account. If you don't have one, you can create it [here](https://github.com/signup){target="_blank"}. Be sure to use a good username, your website will be `<username>.github.io`. 

--- 

* After creating your account, you can create a new repository, you can use this [link](https://github.com/new){target="_blank"}. Be sure to use a good name for your repository, your patch will be available at `<username>.github.io/<repository-name>`. After all configuration, click on the `Create repository` button.

---

* After creating your repository, you will see a weird page with a lot of information. You can ignore all of this and search for the link `uploading an existing file`. Click on it.

<p align="center">
  <img src="../upload.png" alt="github-upload" width="100%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);">
</p>

---
    
* After clicking on the link, you will see a page with a box to drag and drop your files. Upload all files inside the `Webpatch` folder. You can select all files and drag them to the box. After that, you can click on the `Commit changes` button.

---
<p align="center">
  Wait a few seconds to see your files on the repository.
</p>

### Publish your patch

* From your repository page, click on the `Settings` tab. After that, scroll down to the `Pages` section. On the section `Build and deployment`, select the branch `main` and the folder `/root`. After that, click on the `Save` button. 

Wait a few seconds and you will see a link to your patch. You can access your patch using the link `<username>.github.io/<repository-name>`.

!!! bug "Save button not working?"
    Sometimes the `Save` button does not become available for the `/root`, change from `main` to `None` then `Save`, after that, change back to `main` and `Save` again.
    

