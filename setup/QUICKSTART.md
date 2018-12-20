#Quick start

For basic concepts to understand the tool, please visit the <a href="../CONCEPTS.html">Concepts</a> chapter.  
Here we give line-by-line instructions on how to build a first project and explain the various steps to producing an HLS4ML IP.

##Running the tool

These instructions are for simply running the tool out-of-the-box and getting a feel for the workflow.  

```
git clone https://github.com/hls-fpga-machine-learning/HLS4ML.git -b v0.1.3
cd HLS4ML/keras-to-hls
python keras-to-hls.py -c keras-config.yml
```

This will create a new HLS project directory with an implementation of a model from the `example-keras-model-files` directory.
The model files, along with other configuration parameters, are defined in the `keras-config.yml` file.
To run the HLS project, do:

```
cd my-hls-dir-test
vivado_hls -f build_prj.tcl
```

This will create a Vivado HLS project with your model implmentation!

##Existing examples and models

Other examples of various HLS projects with examples of different machine learning algorithm implementations is in the directory: `example-prjs`.

Example [Keras](https://github.com/hls-fpga-machine-learning/hls4ml/tree/master/keras-to-hls/example-keras-model-files) and [PyTorch](https://github.com/hls-fpga-machine-learning/hls4ml/tree/master/pytorch-to-hls/example-models) models are availble.  Here, we also give a little information in what is given in these models.