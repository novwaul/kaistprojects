import tensorflow as tf
import numpy as np

idx2char = ['h','i','e','l','o']

x_data = [[0,1,0,2,3,3]] #hihell
x_one_hot = [[[1,0,0,0,0],[0,1,0,0,0],[1,0,0,0,0],[0,0,1,0,0],[0,0,0,1,0],[0,0,0,1,0]]]
y_data = [[1,0,2,3,3,4]] #ihello

X = tf.placeholder(tf.float32, [None, 6, 5])
Y = tf.placeholder(tf.int32, [None, 6])

cell = tf.nn.rnn_cell.LSTMCell(num_units= 5)
initial_state = cell.zero_state(1, tf.float32)
outputs, _states = tf.nn.dynamic_rnn(cell, X, initial_state= initial_state, dtype= tf.float32)

loss = tf.contrib.seq2seq.sequence_loss(logits= outputs, targets= Y, weights= tf.ones([1, 6]))

train = tf.train.AdamOptimizer(learning_rate = 0.1).minimize(loss)

prediction = tf.argmax(outputs, axis=2)

with tf.Session() as sess:
   sess.run(tf.global_variables_initializer())
   for i in range(2000):
       l, _ = sess.run([loss, train], feed_dict={X: x_one_hot, Y: y_data})
       result = sess.run(prediction, feed_dict={X: x_one_hot})
       print(i, "loss:", l, "prediction: ", result, "true Y: ", y_data)

       # print char using dic
       result_str = [idx2char[c] for c in np.squeeze(result)]
       print("\tPrediction str: ", ''.join(result_str))
