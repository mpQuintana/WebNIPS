import xml.dom.minidom


'''
    Parse xml doc and print into .txt file.
    Each line in the format <StageNumber, StageT, T, lVal, rVal,
                             rect_k[0], rect_k[1], rect_k[2], rect_k[3], rect_k[4]>
'''

cascade_filename = "haarcascade_frontalface_default"
path2Source = "/Users/cipriancorneanu/Research/QtProjects/QtNeurochild/QtNeurochildTest/" + cascade_filename + ".xml"
output_filename = "haar.h"

testSource = """\
<opencv_storage>
<haarcascade_profileface type_id="opencv-haar-classifier">
  <size>20 20</size>
  <stages>
    <_>
      <!-- stage 0 -->
      <trees>
        <_>
          <!-- tree 0 -->
          <_>
            <!-- root node -->
            <feature>
              <rects>
                <_>8 7 2 6 -1.</_>
                <_>8 10 2 3 2.</_></rects>
              <tilted>0</tilted></feature>
            <threshold>1.1384399840608239e-003</threshold>
            <left_val>-0.8377197980880737</left_val>
            <right_val>-0.6608840823173523</right_val></_></_></trees>
      <stage_threshold>-1.1856809854507446</stage_threshold>
      <parent>-1</parent>
      <next>-1</next>
    </_>
    <_>
  <!-- stage 1 -->
  <trees>
    <_>
      <!-- tree 0 -->
      <_>
        <!-- root node -->
        <feature>
          <rects>
            <_>10 4 8 8 -1.</_>
            <_>14 4 4 8 2.</_></rects>
          <tilted>0</tilted></feature>
        <threshold>-0.0195538699626923</threshold>
        <left_val>0.4924583137035370</left_val>
        <right_val>-0.6339616775512695</right_val></_></_>
    <_>
      <!-- tree 1 -->
      <_>
        <!-- root node -->
        <feature>
          <rects>
            <_>5 7 5 4 -1.</_>
            <_>5 9 5 2 2.</_></rects>
          <tilted>0</tilted></feature>
        <threshold>2.2794529795646667e-003</threshold>
        <left_val>-0.6460496783256531</left_val>
        <right_val>0.3581846058368683</right_val></_></_>
    <_>
      <!-- tree 2 -->
      <_>
        <!-- root node -->
        <feature>
          <rects>
            <_>8 4 6 6 -1.</_>
            <_>8 4 3 3 2.</_>
            <_>11 7 3 3 2.</_></rects>
          <tilted>0</tilted></feature>
        <threshold>2.4270440917462111e-003</threshold>
        <left_val>-0.4725323021411896</left_val>
        <right_val>0.2849431037902832</right_val></_></_>
    <_>
      <!-- tree 3 -->
      <_>
        <!-- root node -->
        <feature>
          <rects>
            <_>10 14 5 2 -1.</_>
            <_>10 15 5 1 2.</_></rects>
          <tilted>0</tilted></feature>
        <threshold>1.9644061103463173e-003</threshold>
        <left_val>0.1699953973293304</left_val>
        <right_val>-0.7786815762519836</right_val></_></_>
    <_>
      <!-- tree 4 -->
      <_>
        <!-- root node -->
        <feature>
          <rects>
            <_>7 11 8 4 -1.</_>
            <_>7 13 8 2 2.</_></rects>
          <tilted>0</tilted></feature>
        <threshold>2.2895270958542824e-003</threshold>
        <left_val>0.1555171012878418</left_val>
        <right_val>-0.6672509908676148</right_val></_></_>
    <_>
      <!-- tree 5 -->
      <_>
        <!-- root node -->
        <feature>
          <rects>
            <_>11 14 3 3 -1.</_>
            <_>11 15 3 1 3.</_></rects>
          <tilted>0</tilted></feature>
        <threshold>-3.0143910553306341e-003</threshold>
        <left_val>-0.6872130036354065</left_val>
        <right_val>0.1460456997156143</right_val></_></_>
        </trees>
  <stage_threshold>-1.4913179874420166</stage_threshold>
  <parent>0</parent>
      <next>-1</next></_>
  </stages>
</haarcascade_profileface>
</opencv_storage>
"""


def getText(nodelist):
    rc = []
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            rc.append(node.data)
    return ''.join(rc)


def parseFile(storage):
    # Create empty file
    open('cascade.h', 'w').close()

    # Header
    with open(output_filename, "a") as file:
        file.write('#ifndef CASCADE' + '\n')
        file.write('#define CASCADE' + '\n\n')

    cascade = storage.getElementsByTagName(cascade_filename)

    for node in cascade:
        size = node.getElementsByTagName("size")[0]

        with open(output_filename, "a") as file:
            sizeString = getText(size.childNodes)
            tokens = sizeString.split(' ')
            file.write('const int HAAR_WIDTH = ' + tokens[0] + ';\n')
            file.write('const int HAAR_HEIGHT = ' + tokens[1] + ';\n\n')

            file.write('double haar_data1[][20] = { ')

        stages = node.getElementsByTagName("stages")
        handleStages(stages, "")

    # Footer
    with open(output_filename, "a") as file:
        file.write('};' + '\n' + '#endif' + '\n')


def handleStages(stages, paramStage):
    n = 0

    for stage in stages:
        stageParts = stage.childNodes


        for stagePart in stageParts:
            print n

            if stagePart.nodeType == stagePart.ELEMENT_NODE:
                sT = stagePart.getElementsByTagName("stage_threshold")

                # Handle stage
                paramStage = `n` + ', ' + getText(sT[0].childNodes) + ', '
                handleStage(stagePart, paramStage)
                n = n + 1


def handleStage(stage, paramStage):
    trees = stage.getElementsByTagName("trees")
    handleTrees(trees, paramStage)


def handleTrees(trees, paramStage):
    for tree in trees:
        paramTree = handleTree(tree, paramStage)


def handleTree(tree, paramStage):
    features = tree.getElementsByTagName("feature")
    thresholds = tree.getElementsByTagName("threshold")
    lValues = tree.getElementsByTagName("left_val")
    rValues = tree.getElementsByTagName("right_val")

    for f, t, lV, rV in zip(features, thresholds, lValues, rValues):
        paramFeature = handleFeature(f)

        paramTree = getText(t.childNodes) + ', ' + getText(lV.childNodes) + ', ' + getText(rV.childNodes) + ', '

        # Print line to file
        with open(output_filename, "a") as file:
            file.write('{' + paramStage + paramTree + paramFeature + '},' + '\n')


def handleFeature(feature):
    rects = feature.getElementsByTagName("rects")

    for rect in rects:
        rs = rect.getElementsByTagName("_")

        rect_str = ""
        for r in rs:
            # Print rectangles
            rect_str += getText(r.childNodes) + ' '

    rect_str = rect_str[:-1].replace(' ', ', ')
    tokens = rect_str.split(', ')

    if len(tokens)==10:
        rect_str = rect_str + ', 0, 0, 0, 0, 0'

    return " " + rect_str


# Define source
#source = xml.dom.minidom.parseString(testSource)
source = xml.dom.minidom.parse(path2Source)


parseFile(source)
